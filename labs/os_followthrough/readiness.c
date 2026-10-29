#define _GNU_SOURCE
#include "oslab.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#define CHECK(x)                                                               \
    do                                                                         \
    {                                                                          \
        if (!(x))                                                              \
        {                                                                      \
            fprintf(stderr, "OF_READY:%d:%s errno=%d\n", __LINE__, #x, errno); \
            return 1;                                                          \
        }                                                                      \
    } while (0)
static volatile sig_atomic_t signals;

static void notice(int n)
{
    (void)n;
    signals++;
}

struct of_reader
{
    unsigned char bytes[8];
    size_t used;
    unsigned complete, rejected;
};

static int step(struct of_reader *r, int fd)
{
    if (r->complete || r->rejected)
    {
        return -2;
    }
    ssize_t n = read(fd, r->bytes + r->used, sizeof r->bytes - r->used);
    if (n > 0)
    {
        r->used += (size_t)n;
        if (r->used == sizeof r->bytes)
        {
            r->complete = 1;
        }
        return 1;
    }
    if (n == 0)
    {
        r->rejected = r->used != 0;
        return 0;
    }
    if (errno == EAGAIN || errno == EINTR)
    {
        return 2;
    }
    return -1;
}

int main(void)
{
    int p[2];
    CHECK(pipe2(p, O_CLOEXEC | O_NONBLOCK) == 0);
    struct pollfd poller = {p[0], POLLIN, 0};
    CHECK(write(p[1], "x", 1) == 1 && poll(&poller, 1, 1000) == 1);
    int other = dup(p[0]);
    CHECK(other >= 0);
    char c;
    CHECK(read(other, &c, 1) == 1 && c == 'x');
    struct of_reader r = {0};
    CHECK(step(&r, p[0]) == 2 && r.used == 0);
    CHECK(close(other) == 0);
    CHECK(write(p[1], "abcd", 4) == 4 && poll(&poller, 1, 1000) == 1 &&
          step(&r, p[0]) == 1 && r.used == 4 && !r.complete);
    CHECK(step(&r, p[0]) == 2 && r.used == 4);
    CHECK(write(p[1], "efgh", 4) == 4 && step(&r, p[0]) == 1 && r.complete &&
          !memcmp(r.bytes, "abcdefgh", 8));
    CHECK(close(p[0]) == 0 && close(p[1]) == 0);
    CHECK(pipe2(p, O_CLOEXEC | O_NONBLOCK) == 0);
    r = (struct of_reader){0};
    CHECK(write(p[1], "ij", 2) == 2 && close(p[1]) == 0 &&
          step(&r, p[0]) == 1 && r.used == 2 && step(&r, p[0]) == 0 &&
          r.rejected && !r.complete && close(p[0]) == 0);
    struct sigaction action = {0};
    action.sa_handler = notice;
    CHECK(sigemptyset(&action.sa_mask) == 0 &&
          sigaction(SIGUSR1, &action, NULL) == 0);
    sigset_t blocked, old;
    CHECK(sigemptyset(&blocked) == 0 && sigaddset(&blocked, SIGUSR1) == 0 &&
          sigprocmask(SIG_BLOCK, &blocked, &old) == 0);
    pid_t child = fork();
    CHECK(child >= 0);
    if (child == 0)
    {
        _exit(kill(getppid(), SIGUSR1) == 0 ? 0 : 1);
    }
    struct timespec limit = {1, 0};
    errno = 0;
    CHECK(ppoll(NULL, 0, &limit, &old) == -1 && errno == EINTR && signals == 1);
    int status;
    CHECK(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
          WEXITSTATUS(status) == 0 &&
          sigprocmask(SIG_SETMASK, &old, NULL) == 0);
    CHECK(os_signal_probe() == 0);
    puts("OF_PASS:readiness:stolen-ready-EAGAIN:4+4-record:partial2-EOF-reject:"
         "atomic-unmask-EINTR:actual-selfpipe");
    return 0;
}
