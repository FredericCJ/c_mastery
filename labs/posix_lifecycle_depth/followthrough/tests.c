#define _GNU_SOURCE
#include "launch.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#define CHECK(x)                                                \
    do                                                          \
    {                                                           \
        if (!(x))                                               \
        {                                                       \
            fprintf(stderr, "PLX_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                           \
        }                                                       \
    } while (0)
#ifdef PLX_INJECT
int plx_injection_observed(void);
#endif
static int descriptors(void)
{
    DIR *d = opendir("/proc/self/fd");
    if (!d)
    {
        return -1;
    }
    int n = 0;
    while (readdir(d))
    {
        ++n;
    }
    closedir(d);
    return n;
}

static void reap(pid_t p)
{
    int s;
    while (waitpid(p, &s, 0) < 0 && errno == EINTR)
    {
    }
}

static int channel(const char *helper)
{
    char *args[] = {(char *)helper, "zero", NULL};
    struct plx_result r = {9, 9, 9};
    int before = descriptors();
    CHECK(before > 0);
    for (unsigned chunk = 0; chunk <= 8; ++chunk)
    {
        CHECK(plx_launch(helper, args, (struct plx_options){PLX_NORMAL, chunk},
                         &r) == 0);
        CHECK(r.kind == PLX_EXITED && r.value == 0 && r.stage == 0);
        args[1] = "127";
        CHECK(plx_launch(helper, args, (struct plx_options){PLX_NORMAL, chunk},
                         &r) == 0);
        CHECK(r.kind == PLX_EXITED && r.value == 127 && r.stage == 0);
        args[1] = "zero";
        char missing[4096];
        int n = snprintf(missing, sizeof missing, "%s.absent", helper);
        CHECK(n > 0 && (size_t)n < sizeof missing);
        CHECK(access(missing, F_OK) == -1 && errno == ENOENT);
        CHECK(plx_launch(missing, args, (struct plx_options){PLX_NORMAL, chunk},
                         &r) == 0);
        CHECK(r.kind == PLX_FAILED && r.value == ENOENT && r.stage == 2);
        r = (struct plx_result){9, 9, 9};
        CHECK(plx_launch(helper, args,
                         (struct plx_options){PLX_SHORT_RECORD, chunk},
                         &r) == EPROTO);
        CHECK(r.kind == 9 && r.value == 9 && r.stage == 9);
    }
    CHECK(plx_launch(helper, args, (struct plx_options){PLX_NORMAL, 9}, &r) ==
          EINVAL);
#ifdef PLX_INJECT
    CHECK(plx_injection_observed());
#endif
    CHECK(descriptors() == before);
    CHECK(waitpid(-1, NULL, WNOHANG) == -1 && errno == ECHILD);
    return 0;
}

static int stages(const char *helper)
{
    char *args[] = {(char *)helper, "zero", NULL};
    struct plx_result r = {9, 9, 9};
    int before = descriptors();
    CHECK(plx_launch(helper, args, (struct plx_options){PLX_SETUP_FAILURE, 1},
                     &r) == 0);
    CHECK(r.kind == PLX_FAILED && r.value == EACCES && r.stage == 1);
    r = (struct plx_result){9, 9, 9};
    CHECK(plx_launch(helper, args, (struct plx_options){PLX_UNKNOWN_STAGE, 1},
                     &r) == EPROTO);
    CHECK(r.kind == 9 && r.value == 9);
    CHECK(plx_launch(helper, args, (struct plx_options){PLX_EARLY_EXIT, 1},
                     &r) == 0);
    CHECK(r.kind == PLX_EXITED && r.value == 126);
    CHECK(plx_launch(helper, args, (struct plx_options){PLX_EARLY_SIGNAL, 1},
                     &r) == 0);
    CHECK(r.kind == PLX_SIGNALED && r.value == SIGTERM);
    args[1] = "signal";
    CHECK(plx_launch(helper, args, (struct plx_options){PLX_NORMAL, 1}, &r) ==
          0);
    CHECK(r.kind == PLX_SIGNALED && r.value == SIGTERM);
    CHECK(descriptors() == before);
    CHECK(waitpid(-1, NULL, WNOHANG) == -1 && errno == ECHILD);
    return 0;
}

static int redirect(void)
{
    int initial = descriptors();
    for (unsigned crossing = 0; crossing < 2; ++crossing)
    {
        for (unsigned fail = 0; fail <= 4; ++fail)
        {
            char name[] = "/tmp/plx-owned-XXXXXX";
            int a = mkstemp(name);
            CHECK(a >= 0);
            CHECK(unlink(name) == 0);
            char other[] = "/tmp/plx-owned-XXXXXX";
            int b = mkstemp(other);
            CHECK(b >= 0);
            CHECK(unlink(other) == 0);
            CHECK(write(a, "ABC", 3) == 3 && write(b, "xyz", 3) == 3);
            CHECK(lseek(a, 0, SEEK_SET) == 0 && lseek(b, 0, SEEK_SET) == 0);
            CHECK(fcntl(a, F_SETFD, FD_CLOEXEC) == 0 &&
                  fcntl(b, F_SETFD, FD_CLOEXEC) == 0);
            int s[2] = {a, b}, d[2] = {crossing ? b : a, crossing ? a : b};
            int before = descriptors();
            int rc = plx_redirect(s, d, fail);
            CHECK(rc == (fail ? EIO : 0));
            CHECK(descriptors() == before);
            char first = '?', second = '?';
            CHECK(read(d[0], &first, 1) == 1 && read(d[1], &second, 1) == 1);
            if (!fail)
            {
                CHECK(first == 'A' && second == 'x');
                CHECK(!(fcntl(a, F_GETFD) & FD_CLOEXEC) &&
                      !(fcntl(b, F_GETFD) & FD_CLOEXEC));
            }
            if (fail <= 2 && fail)
            {
                CHECK(crossing ? (first == 'x' && second == 'A')
                               : (first == 'A' && second == 'x'));
            }
            CHECK(close(a) == 0 && close(b) == 0);
        }
    }
    CHECK(descriptors() == initial);
    return 0;
}

static int inheritance(void)
{
    CHECK(plx_tree(-1, -1, -1, 0) == EINVAL);
    int initial = descriptors();
    for (int repair = 0; repair < 2; ++repair)
    {
        int data[2], release[2], ready[2];
        CHECK(pipe2(data, O_CLOEXEC | O_NONBLOCK) == 0);
        CHECK(pipe2(release, O_CLOEXEC) == 0);
        CHECK(pipe2(ready, O_CLOEXEC) == 0);
        pid_t child = fork();
        CHECK(child >= 0);
        if (child == 0)
        {
            close(data[0]);
            close(release[1]);
            close(ready[0]);
            int e = plx_tree(data[1], release[0], ready[1], repair);
            _exit(e ? 123 : 0);
        }
        close(data[1]);
        close(release[0]);
        close(ready[1]);
        char token = '?';
        ssize_t ready_count;
        do
        {
            ready_count = read(ready[0], &token, 1);
        } while (ready_count < 0 && errno == EINTR);
        /* EOF from ready proves the intermediate helper closed its own writer too. */
        char extra;
        ssize_t ready_eof;
        do
        {
            ready_eof = read(ready[0], &extra, 1);
        } while (ready_eof < 0 && errno == EINTR);
        errno = 0;
        ssize_t count = read(data[0], &extra, 1);
        int error = errno;
        /* Always release and reap owned processes before checking the observation. */
        ssize_t released = write(release[1], "G", 1);
        close(release[1]);
        close(ready[0]);
        int status = 0;
        pid_t got;
        do
        {
            got = waitpid(child, &status, 0);
        } while (got < 0 && errno == EINTR);
        ssize_t final = read(data[0], &extra, 1);
        close(data[0]);
        CHECK(ready_count == 1 && token == 'R' && ready_eof == 0 &&
              released == 1);
        CHECK(got == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
        CHECK(repair
                  ? count == 0
                  : (count == -1 && (error == EAGAIN || error == EWOULDBLOCK)));
        CHECK(final == 0 && descriptors() == initial);
    }
    CHECK(waitpid(-1, NULL, WNOHANG) == -1 && errno == ECHILD);
    return 0;
}

static volatile sig_atomic_t signal_seen;

static void notice(int signal_number)
{
    (void)signal_number;
    signal_seen = 1;
}

static void pause_child(void)
{
    struct timespec t = {0, 100000000};
    while (nanosleep(&t, &t) < 0 && errno == EINTR)
    {
    }
}

/* The three blocking calls under test, each interrupted by a periodic
   SIGALRM while the child delays its side by ~100 ms. */
enum
{
    INTERRUPT_READ_ALL,
    INTERRUPT_WRITE_ALL,
    INTERRUPT_WAIT,
    INTERRUPT_MODES
};

/* Fills a non-blocking pipe until it would block, so the next write must
   wait for the child to read. *filled receives the byte count. */
static int fill_pipe(int write_end, size_t *filled)
{
    char bytes[256] = {0};
    *filled = 0;
    for (;;)
    {
        ssize_t n = write(write_end, bytes, sizeof bytes);
        if (n > 0)
        {
            *filled += (size_t)n;
        }
        else
        {
            CHECK(n == -1 && errno == EAGAIN);
            return 0;
        }
    }
}

/* Child for INTERRUPT_READ_ALL: after a pause, delivers 8 bytes one at a
   time, so the parent's read is interrupted and resumed repeatedly. */
static void feed_reader(const int pipefd[2])
{
    close(pipefd[0]);
    pause_child();
    for (unsigned i = 0; i < 8; ++i)
    {
        if (write(pipefd[1], &"abcdefgh"[i], 1) != 1)
        {
            _exit(122);
        }
    }
    close(pipefd[1]);
}

/* Child for INTERRUPT_WRITE_ALL: after a pause, drains the full pipe plus
   the parent's one extra byte. The read end is non-blocking, so EAGAIN is
   retried rather than treated as the end. */
static void drain_pipe(const int pipefd[2], size_t filled)
{
    close(pipefd[1]);
    pause_child();
    char bytes[256];
    size_t drained = 0;
    while (drained < filled + 1)
    {
        ssize_t n = read(pipefd[0], bytes, sizeof bytes);
        if (n > 0)
        {
            drained += (size_t)n;
        }
        else if (n < 0 && (errno == EINTR || errno == EAGAIN))
        {
            continue;
        }
        else
        {
            break;
        }
    }
    close(pipefd[0]);
}

/* Child body per mode; for INTERRUPT_WAIT it only sleeps and exits 0. */
static void run_interrupt_child(unsigned mode, const int pipefd[2],
                                size_t filled)
{
    if (mode == INTERRUPT_READ_ALL)
    {
        feed_reader(pipefd);
    }
    else if (mode == INTERRUPT_WRITE_ALL)
    {
        drain_pipe(pipefd, filled);
    }
    else
    {
        close(pipefd[0]);
        close(pipefd[1]);
        pause_child();
    }
    _exit(0);
}

/* The parent keeps only the end its call uses. */
static void close_unused_ends(unsigned mode, const int pipefd[2])
{
    if (mode == INTERRUPT_READ_ALL)
    {
        close(pipefd[1]);
    }
    else if (mode == INTERRUPT_WRITE_ALL)
    {
        close(pipefd[0]);
    }
    else
    {
        close(pipefd[0]);
        close(pipefd[1]);
    }
}

static int interrupts(void)
{
    struct sigaction action = {0};
    struct sigaction old;
    action.sa_handler = notice;
    CHECK(sigemptyset(&action.sa_mask) == 0);
    CHECK(sigaction(SIGALRM, &action, &old) == 0);
    /* 1000 microsecond period: frequent enough to hit every blocked call. */
    struct itimerval timer = {{0, 1000}, {0, 1000}};
    struct itimerval none = {{0, 0}, {0, 0}};
    struct itimerval previous;
    CHECK(getitimer(ITIMER_REAL, &previous) == 0 &&
          previous.it_value.tv_sec == 0 && previous.it_value.tv_usec == 0);
    int initial = descriptors();
    for (unsigned mode = 0; mode < INTERRUPT_MODES; ++mode)
    {
        int pipefd[2];
        int nonblocking = mode == INTERRUPT_WRITE_ALL ? O_NONBLOCK : 0;
        CHECK(pipe2(pipefd, O_CLOEXEC | nonblocking) == 0);
        size_t filled = 0;
        if (mode == INTERRUPT_WRITE_ALL)
        {
            CHECK(fill_pipe(pipefd[1], &filled) == 0);
            /* Back to blocking, so the call under test really blocks. */
            CHECK(fcntl(pipefd[1], F_SETFL, 0) == 0);
        }
        pid_t child = fork();
        CHECK(child >= 0);
        if (child == 0)
        {
            run_interrupt_child(mode, pipefd, filled);
        }
        close_unused_ends(mode, pipefd);

        signal_seen = 0;
        CHECK(setitimer(ITIMER_REAL, &timer, NULL) == 0);
        int status = 0;
        int rc;
        unsigned interrupted = 0;
        size_t used = 0;
        char bytes[8] = {0};
        if (mode == INTERRUPT_READ_ALL)
        {
            rc = plx_read_all(pipefd[0], bytes, 8, &used, &interrupted);
        }
        else if (mode == INTERRUPT_WRITE_ALL)
        {
            rc = plx_write_all(pipefd[1], "!", 1, &used, &interrupted);
        }
        else
        {
            rc = plx_wait(child, &status, &interrupted);
        }
        CHECK(setitimer(ITIMER_REAL, &none, NULL) == 0);
        if (mode != INTERRUPT_WAIT)
        {
            close(pipefd[mode == INTERRUPT_READ_ALL ? 0 : 1]);
        }
        /* Reap here unless plx_wait already did so successfully. */
        if (mode != INTERRUPT_WAIT || rc)
        {
            reap(child);
        }
        CHECK(rc == 0 && interrupted > 0 && signal_seen);
        if (mode == INTERRUPT_READ_ALL)
        {
            CHECK(used == 8 && memcmp(bytes, "abcdefgh", 8) == 0);
        }
        if (mode == INTERRUPT_WRITE_ALL)
        {
            CHECK(used == 1);
        }
        if (mode == INTERRUPT_WAIT)
        {
            CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0);
        }
        CHECK(descriptors() == initial);
        printf("EINTR mode=%u observed=%u\n", mode, interrupted);
    }
    CHECK(sigaction(SIGALRM, &old, NULL) == 0);
    CHECK(waitpid(-1, NULL, WNOHANG) == -1 && errno == ECHILD);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        return 2;
    }
    int rc;
    if (!strcmp(argv[1], "channel"))
    {
        rc = channel(argv[2]);
    }
    else if (!strcmp(argv[1], "stages"))
    {
        rc = stages(argv[2]);
    }
    else if (!strcmp(argv[1], "redirect"))
    {
        rc = redirect();
    }
    else if (!strcmp(argv[1], "inheritance"))
    {
        rc = inheritance();
    }
    else if (!strcmp(argv[1], "interrupts"))
    {
        rc = interrupts();
    }
    else
    {
        return 2;
    }
    if (!rc)
    {
        printf("PLX_PASS %s\n", argv[1]);
    }
    return rc;
}
