#include "ipc.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

struct rd_port
{
    int rx, tx;
    unsigned waits;
};

static int now(void *p, struct timespec *t)
{
    (void)p;
    return clock_gettime(CLOCK_MONOTONIC, t);
}

static ssize_t transfer(void *p, int sending, unsigned char *b, size_t n)
{
    struct rd_port *port = p;
    return sending ? write(port->tx, b, n) : read(port->rx, b, n);
}

static int wait_ready(void *p, int sending, struct timespec left)
{
    struct rd_port *port = p;
    ++port->waits;
    struct pollfd fd = {sending ? port->tx : port->rx,
                        sending ? POLLOUT : POLLIN, 0};
    return ppoll(&fd, 1, &left, 0);
}

#define CHECK(c)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(c))                                                           \
        {                                                                   \
            fprintf(stderr, "IPC-BUDGET-NATIVE-CHECK:%s:%d errno=%d\n", #c, \
                    __LINE__, errno);                                       \
            return 1;                                                       \
        }                                                                   \
    } while (0)

int main(void)
{
    alarm(6);
    int request[2], response[2], gate[2], ready[2];
    CHECK(pipe2(request, O_CLOEXEC | O_NONBLOCK) == 0 &&
          pipe2(response, O_CLOEXEC | O_NONBLOCK) == 0 &&
          pipe2(gate, O_CLOEXEC) == 0 && pipe2(ready, O_CLOEXEC) == 0);
    pid_t child = fork();
    CHECK(child >= 0);
    if (!child)
    {
        alarm(5);
        close(request[1]);
        close(response[0]);
        close(gate[1]);
        close(ready[0]);
        if (write(response[1], "AB", 2) != 2 || write(ready[1], "R", 1) != 1)
            _exit(2);
        char release;
        if (read(gate[0], &release, 1) != 1)
            _exit(3);
        unsigned char bytes[8];
        if (read(request[0], bytes, 8) != 8)
            _exit(4);
        close(request[0]);
        close(response[1]);
        close(gate[0]);
        close(ready[1]);
        _exit(0);
    }
    close(request[0]);
    close(response[1]);
    close(gate[0]);
    close(ready[1]);
    char enrolled;
    CHECK(read(ready[0], &enrolled, 1) == 1 && enrolled == 'R');
    close(ready[0]);
    struct rd_port port = {response[0], request[1], 0};
    struct ei_io ops = {now, transfer, wait_ready, &port};
    struct ei_progress p = {0};
    const unsigned char request_bytes[8] = {0, 1, 2, 3, 4, 5, 6, 7};
    unsigned char output[4] = {99, 99, 99, 99};
    struct timespec begin, end, deadline;
    CHECK(clock_gettime(CLOCK_MONOTONIC, &begin) == 0);
    deadline = begin;
    deadline.tv_nsec += 50000000L;
    if (deadline.tv_nsec >= 1000000000L)
    {
        ++deadline.tv_sec;
        deadline.tv_nsec -= 1000000000L;
    }
    CHECK(ei_exchange(ops, request_bytes, output, deadline, &p) == -1 &&
          errno == ETIMEDOUT && p.sent == 8 && p.received == 2 &&
          p.reply[0] == 'A' && p.reply[1] == 'B' && output[0] == 99 &&
          port.waits >= 1);
    CHECK(clock_gettime(CLOCK_MONOTONIC, &end) == 0);
    CHECK(write(gate[1], "Q", 1) == 1);
    close(gate[1]);
    close(request[1]);
    close(response[0]);
    int status;
    CHECK(waitpid(child, &status, 0) == child && WIFEXITED(status) &&
          WEXITSTATUS(status) == 0);
    printf(
        "IPC-BUDGET-NATIVE:sent=8;partial_reply=2;timeout=1;waits=%u;elapsed_"
        "seconds=%.9f;hard_latency_guarantee=0;\n",
        port.waits,
        (double)(end.tv_sec - begin.tv_sec) +
            (double)(end.tv_nsec - begin.tv_nsec) / 1e9);
    return 0;
}
