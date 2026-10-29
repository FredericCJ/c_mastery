#define _POSIX_C_SOURCE 200809L
#include "dispatch_broker.h"
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

/* No socket names, listeners, external addresses or host-security changes. */
static int ready(int fd, short events)
{
    struct pollfd p = {fd, events, 0};
    int rc;
    do
    {
        rc = poll(&p, 1, 2000);
    } while (rc < 0 && errno == EINTR);
    return rc > 0 && (p.revents & (events | POLLHUP)) &&
           !(p.revents & (POLLERR | POLLNVAL));
}

static int send_bytes(int fd, const unsigned char *p, unsigned n, unsigned cap)
{
    unsigned done = 0;
    while (done < n)
    {
        if (!ready(fd, POLLOUT))
            return 0;
        unsigned count = n - done;
        if (count > cap)
            count = cap;
        ssize_t got = send(fd, p + done, count, MSG_DONTWAIT | MSG_NOSIGNAL);
        if (got > 0)
            done += (unsigned)got;
        else if (got < 0 &&
                 (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
            continue;
        else
            return 0;
    }
    return 1;
}

/* 1 complete frame; 0 clean EOF; -1 truncated EOF or I/O timeout/failure. */
static int receive_frame(int fd, unsigned char p[16], unsigned cap,
                         unsigned *partial)
{
    unsigned done = 0;
    while (done < 16)
    {
        if (!ready(fd, POLLIN))
            return -1;
        unsigned count = 16 - done;
        if (count > cap)
            count = cap;
        ssize_t got = recv(fd, p + done, count, MSG_DONTWAIT);
        if (got > 0)
            done += (unsigned)got;
        else if (got == 0)
        {
            *partial = done;
            return done ? -1 : 0;
        }
        else if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            continue;
        else
            return -1;
    }
    return 1;
}

static int client(int fd)
{
    const unsigned operations[11] = {1, 2, 2, 2, 4, 1, 3, 2, 1, 1, 1};
    const unsigned target[11] = {0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0};
    const unsigned capid[11] = {1, 1, 1, 2, 1, 3, 1, 4, 1, 1, 1};
    const unsigned sequence[11] = {1, 2, 2, 3, 4, 5, 6, 7, 8, 9, 9};
    const unsigned status[11] = {DS_OK,     DS_OK,      DS_STALE, DS_DENIED,
                                 DS_DENIED, DS_DENIED,  DS_OK,    DS_DENIED,
                                 DS_STALE,  DS_INVALID, DS_STALE};
    for (unsigned i = 0; i < 11; ++i)
    {
        unsigned char request[16], reply[16];
        unsigned value = operations[i] == 2 || operations[i] == 4
                             ? (i == 1 || i == 2 ? 5u : 7u)
                             : 0;
        ds_broker_request(request, operations[i], target[i], i == 6 ? 1 : 0,
                          capid[i], sequence[i], 1, value);
        if (i == 9)
            request[0] = 99;
        if (!send_bytes(fd, request, 16, 1 + i % 7))
            return 1;
        unsigned partial = 0;
        if (receive_frame(fd, reply, 5, &partial) != 1 || reply[1] != status[i])
            return 1;
        if (i == 1 && ds_broker_u32(reply + 12) != 5)
            return 1;
        if (i == 6 && ds_broker_u32(reply + 4) != 4)
            return 1;
        if (status[i] != DS_OK && ds_broker_u32(reply + 12) != 0)
            return 1;
    }
    unsigned char partial[16];
    ds_broker_request(partial, DS_B_ADD, 0, 0, 1, 10, 1, 7);
    return send_bytes(fd, partial, 5, 1) ? 0 : 1;
}

int ds_broker_process_experiment(void)
{
    int pair[2] = {-1, -1};
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, pair))
        return 1;
    volatile int private_value = 17;
    pid_t child = fork();
    if (child < 0)
    {
        close(pair[0]);
        close(pair[1]);
        return 1;
    }
    if (child == 0)
    {
        close(pair[0]);
        private_value = 999;
        (void)private_value;
        int rc = client(pair[1]);
        close(pair[1]);
        _exit(rc);
    }
    close(pair[1]);
    struct ds_broker authority = {0};
    ds_broker_init(&authority);
    unsigned frames = 0, partial = 0;
    int stream_status = 0;
    for (unsigned i = 0; i < 32; ++i)
    {
        unsigned char request[16], reply[16];
        stream_status = receive_frame(pair[0], request, 3, &partial);
        if (stream_status != 1)
            break;
        if (ds_broker_apply(&authority, 0, request, reply) != DS_OK)
        {
            stream_status = -2;
            break;
        }
        ++frames;
        if (!send_bytes(pair[0], reply, 16, 2))
        {
            stream_status = -3;
            break;
        }
        if (frames == 8 && ds_broker_revoke(&authority, 0) != DS_OK)
        {
            stream_status = -4;
            break;
        }
    }
    close(pair[0]);
    int status = 0;
    pid_t waited;
    do
    {
        waited = waitpid(child, &status, 0);
    } while (waited < 0 && errno == EINTR);
    printf(
        "broker child: frames=%u truncated=%u counters=%u,%u next=%u child_ok=%d parent_private=%d\n",
        frames, partial, authority.counters[0], authority.counters[1],
        authority.next_sequence[0],
        waited == child && WIFEXITED(status) && WEXITSTATUS(status) == 0,
        private_value);
    return !(waited == child && WIFEXITED(status) && WEXITSTATUS(status) == 0 &&
             frames == 11 && partial == 5 && stream_status == -1 &&
             authority.counters[0] == 5 && authority.counters[1] == 100 &&
             authority.next_sequence[0] == 10 && private_value == 17);
}
