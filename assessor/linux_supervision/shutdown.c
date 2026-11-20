#define _GNU_SOURCE
#include "supervision.h"
#include "support.h"

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* Namespace init needs an explicit disposition before its child can send TERM.
 * Blocked delivery and sigtimedwait give the supervisor a synchronous event. */
static void term_handler(int signo) { (void)signo; }

static void install_term(void (*handler)(int))
{
    struct sigaction action = {.sa_handler = handler};
    sigemptyset(&action.sa_mask);
    if (sigaction(SIGTERM, &action, NULL)) fail_sys("sigaction");
}

static int wait_term(const sigset_t *signals, int64_t deadline, siginfo_t *info)
{
    for (;;) {
        int64_t remaining = deadline - now_ms();
        if (remaining <= 0) fail_protocol("TERM deadline expired");
        struct timespec timeout = {
            .tv_sec = remaining / 1000, .tv_nsec = (remaining % 1000) * 1000000
        };
        int signo = sigtimedwait(signals, info, &timeout);
        if (signo < 0 && errno == EINTR) continue;
        if (signo < 0) fail_sys("sigtimedwait");
        return signo;
    }
}

static void run_worker(const char *variant, int writer, const sigset_t *signals,
                       int64_t deadline)
{
    if (!strcmp(variant, "early")) _exit(23);
    if (!strcmp(variant, "startfail")) {
        execl("/no-such-supervision-worker", "worker", (char *)NULL);
        int launch_errno = errno;
        write_full(writer, &launch_errno, sizeof launch_errno, deadline);
        _exit(127);
    }

    int stubborn = !strcmp(variant, "stubborn");
    if (stubborn) {
        install_term(SIG_IGN);
        if (sigprocmask(SIG_UNBLOCK, signals, NULL)) fail_sys("worker unblock");
    }
    int ready = 0;
    write_full(writer, &ready, sizeof ready, deadline);
    /* A real inbound termination request after publication of readiness. */
    if (kill(getppid(), SIGTERM)) fail_sys("request shutdown");
    if (stubborn) for (;;) pause();
    siginfo_t info;
    if (wait_term(signals, deadline, &info) != SIGTERM || info.si_pid != getppid())
        fail_protocol("unexpected worker signal");
    close(writer);
    _exit(0);
}

/* Use a full-duplex control socket so that a worker can acknowledge the EOF
 * caused by closing the parent's write half. Wait for the readiness byte
 * before attempting the next worker's exec. */
static pid_t start_peer(int *control, int64_t deadline)
{
    int channel[2];
    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, channel))
        fail_sys("established-worker channel");
    pid_t peer = fork();
    if (peer < 0) fail_sys("established-worker fork");
    if (!peer) {
        close(channel[0]);
        write_full(channel[1], "R", 1, deadline);
        await_fd(channel[1], POLLIN, deadline);
        char byte;
        if (read(channel[1], &byte, 1) != 0) fail_protocol("expected supervisor EOF");
        write_full(channel[1], "E", 1, deadline);
        close(channel[1]);
        _exit(0);
    }
    close(channel[1]);
    char ready;
    read_full(channel[0], &ready, 1, deadline);
    if (ready != 'R') fail_protocol("established worker never became ready");
    *control = channel[0];
    return peer;
}

int supervise_shutdown(const char *variant)
{
    if (strcmp(variant, "normal") && strcmp(variant, "early") &&
        strcmp(variant, "startfail") && strcmp(variant, "stubborn")) return 2;
    int before = count_fds();
    sigset_t terms;
    sigemptyset(&terms);
    sigaddset(&terms, SIGTERM);
    install_term(term_handler);
    if (sigprocmask(SIG_BLOCK, &terms, NULL)) fail_sys("block TERM");

    int64_t start = now_ms(), deadline = start + 4000;
    int peer_fd = -1;
    pid_t peer = -1;
    if (!strcmp(variant, "startfail")) peer = start_peer(&peer_fd, deadline);
    int status_pipe[2];
    if (pipe2(status_pipe, O_CLOEXEC)) fail_sys("shutdown pipe");
    pid_t child = fork();
    if (child < 0) fail_sys("shutdown fork");
    if (child == 0) {
        if (peer_fd >= 0) close(peer_fd);
        close(status_pipe[0]);
        run_worker(variant, status_pipe[1], &terms, deadline);
    }
    close(status_pipe[1]);
    await_fd(status_pipe[0], POLLIN, deadline);
    int startup_errno = 0;
    ssize_t n = read(status_pipe[0], &startup_errno, sizeof startup_errno);
    if (n < 0) fail_sys("read startup");
    if (n && n != (ssize_t)sizeof startup_errno) fail_protocol("short startup record");
    int ready = n > 0 && !startup_errno;
    int term = 0, escalated = 0, status = 0;
    int64_t grace_ms = 0;
    pid_t waited = 0;
    if (ready) {
        siginfo_t info;
        if (wait_term(&terms, deadline, &info) != SIGTERM || info.si_pid != child)
            fail_protocol("unexpected inbound signal");
        if (kill(child, SIGTERM)) fail_sys("forward TERM");
        term = 1;
        int64_t term_at = now_ms();
        int64_t grace = term_at + 150;
        while (now_ms() < grace) {
            waited = waitpid(child, &status, WNOHANG);
            if (waited == child) break;
            if (waited < 0 && errno != EINTR) fail_sys("grace wait");
            struct timespec tick = {.tv_nsec = 1000000};
            nanosleep(&tick, NULL);
        }
        if (waited != child) {
            grace_ms = now_ms() - term_at;
            if (kill(child, SIGKILL)) fail_sys("forward KILL");
            escalated = 1;
        }
    }
    if (waited != child) {
        status = collect(child, deadline);
        waited = child;
    }
    int reaped = waited == child;
    await_fd(status_pipe[0], POLLIN, deadline);
    char extra;
    ssize_t tail = read(status_pipe[0], &extra, 1);
    if (tail < 0) fail_sys("read status tail");
    close(status_pipe[0]);
    int peer_eof = 0, peer_exit = -1;
    if (peer > 0) {
        if (shutdown(peer_fd, SHUT_WR)) fail_sys("established-worker half-close");
        char acknowledgment;
        read_full(peer_fd, &acknowledgment, 1, deadline);
        peer_eof = acknowledgment == 'E';
        close(peer_fd);
        int peer_status = collect(peer, deadline);
        ++reaped;
        if (WIFEXITED(peer_status)) peer_exit = WEXITSTATUS(peer_status);
    }
    int unused;
    pid_t leftovers = waitpid(-1, &unused, WNOHANG);
    int children_errno = leftovers < 0 ? errno : 0;
    printf("LS shutdown variant=%s ready=%d startup_errno=%d term=%d escalated=%d "
           "exit=%d signal=%d eof=%d reaped=%d children_errno=%d fd_before=%d "
           "fd_after=%d peer_started=%d peer_eof=%d peer_exit=%d grace_ms=%" PRId64 " elapsed_ms=%" PRId64 "\n", variant, ready, startup_errno,
           term, escalated, WIFEXITED(status) ? WEXITSTATUS(status) : -1,
           WIFSIGNALED(status) ? WTERMSIG(status) : 0, tail == 0, reaped, children_errno,
           before, count_fds(), peer > 0, peer_eof, peer_exit, grace_ms, now_ms() - start);
    return 0;
}
