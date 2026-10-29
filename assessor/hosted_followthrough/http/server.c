#define _POSIX_C_SOURCE 200809L
#include "flow.h"
#include "http_depth.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

struct hf_connection
{
    int fd;
    struct hf_handle handle;
    struct hf_flow flow;
    unsigned char input[32];
    size_t have, used;
};

static volatile sig_atomic_t stop;

static void interrupted(int s)
{
    (void)s;
    stop = 1;
}

static uint64_t now_ms(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t))
        abort();
    return (uint64_t)t.tv_sec * 1000 + (unsigned)t.tv_nsec / 1000000;
}

static int flags(int fd)
{
    int a = fcntl(fd, F_GETFL), b = fcntl(fd, F_GETFD);
    return a < 0 || b < 0 || fcntl(fd, F_SETFL, a | O_NONBLOCK) < 0 ||
                   fcntl(fd, F_SETFD, b | FD_CLOEXEC) < 0
               ? -1
               : 0;
}

static int accept_socket(void *v)
{
    int fd = accept(*(int *)v, 0, 0);
    return fd >= 0                                   ? fd
           : errno == EINTR                          ? -1
           : errno == EAGAIN || errno == EWOULDBLOCK ? -2
                                                     : -3;
}

static int accept_flags(void *v, int fd)
{
    (void)v;
    return flags(fd);
}

static void accept_close(void *v, int fd)
{
    (void)v;
    close(fd);
}

/* Acquire into a private local. All five failure sites close exactly once;
   caller's published descriptor/address remain unchanged on failure. */
static int listener_init(unsigned short port, unsigned fault, int *out,
                         struct sockaddr_in *published)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;
    struct sockaddr_in a = {0};
    a.sin_family = AF_INET;
    a.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    a.sin_port = htons(port);
    socklen_t z = sizeof a;
    if (fault == 1 || flags(fd) || fault == 2 ||
        bind(fd, (struct sockaddr *)&a, sizeof a) || fault == 3 ||
        listen(fd, 4) || fault == 4 ||
        getsockname(fd, (struct sockaddr *)&a, &z) || fault == 5)
    {
        close(fd);
        return -1;
    }
    *out = fd;
    *published = a;
    return 0;
}

static const char *utc_date(unsigned mode, char out[30])
{
    if (mode == 0)
        return 0;
    time_t t = mode == 2 ? (time_t)1672531200 : time(0);
    if (t < 0)
        return 0;
    uint64_t seconds = (uint64_t)t;
    if (seconds / 86400 > 2932896)
        return 0;
    size_t n;
    if (hd_date(1, (uint32_t)(seconds / 86400), (uint32_t)(seconds % 86400),
                out, 30, &n))
        return 0;
    return out;
}

static int write_socket(void *v, const void *p, size_t n)
{
    int fd = *(int *)v;
    ssize_t r = send(fd, p, n, MSG_NOSIGNAL);
    return r >= 0                                    ? (int)r
           : errno == EINTR                          ? -1
           : errno == EAGAIN || errno == EWOULDBLOCK ? -2
                                                     : -3;
}

static void release(struct hf_pool *p, struct hf_connection *c)
{
    if (c->fd >= 0)
    {
        int fd = -1;
        if (!hf_close(p, c->handle, &fd))
            close(fd);
        c->fd = -1;
    }
}

int main(int argc, char **argv)
{
    unsigned cap = argc > 1 ? (unsigned)strtoul(argv[1], 0, 10) : 2,
             mode = argc > 2 ? (unsigned)strtoul(argv[2], 0, 10) : 1;
    if (cap < 1 || cap > 2 || mode > 2)
        return 2;
    int listener = -1;
    struct sockaddr_in addr = {0};
    if (listener_init(0, 0, &listener, &addr))
        return 2;
    char authority[128];
    snprintf(authority, sizeof authority, "127.0.0.1:%u", ntohs(addr.sin_port));
    struct hf_pool pool = {0};
    struct hf_connection c[4];
    memset(c, 0, sizeof c);
    for (unsigned i = 0; i < 4; i++)
        c[i].fd = -1;
    signal(SIGINT, interrupted);
    signal(SIGTERM, interrupted);
    printf("READY %s capacity=%u date=%u ledger=%zu flags=%d,%d\n", authority,
           cap, mode, sizeof c + sizeof pool, fcntl(listener, F_GETFL),
           fcntl(listener, F_GETFD));
    fflush(stdout);
    while (!stop)
    {
        struct pollfd f[5] = {{listener, POLLIN, 0}};
        struct hf_handle captured[4];
        for (unsigned i = 0; i < 4; i++)
        {
            short events = 0;
            captured[i] = c[i].handle;
            if (c[i].fd >= 0)
            {
                if (c[i].flow.draining ||
                    (!c[i].flow.closing && c[i].flow.count < cap && !c[i].have))
                    events |= POLLIN;
                if (c[i].flow.count)
                    events |= POLLOUT;
            }
            f[i + 1] = (struct pollfd){c[i].fd, events, 0};
        }
        int ready = poll(f, 5, 20);
        if (ready < 0)
        {
            if (errno == EINTR)
                continue;
            break;
        }
        uint64_t now = now_ms();
        char date[30];
        const char *d = utc_date(mode, date);
        struct hf_handle admitted[4];
        unsigned admissions = 0;
        if (f[0].revents & POLLIN)
        {
            struct hf_accept ops = {accept_socket, accept_flags, accept_close,
                                    &listener};
            (void)hf_accept_batch(&pool, ops, admitted, &admissions);
        }
        for (unsigned a = 0; a < admissions; a++)
        {
            struct hf_handle h = admitted[a];
            int fd = pool.slot[h.index].fd;
            struct hf_connection *s = &c[h.index];
            memset(s, 0, sizeof *s);
            s->fd = fd;
            s->handle = h;
            if (hf_init(&s->flow, cap, authority, now))
            {
                release(&pool, s);
                continue;
            }
            fprintf(stderr, "ACCEPT fd=%d generation=%u flags=%d,%d\n", fd,
                    h.generation, fcntl(fd, F_GETFL), fcntl(fd, F_GETFD));
        }
        for (unsigned i = 0; i < 4; i++)
        {
            struct hf_connection *s = &c[i];
            if (s->fd < 0)
                continue;
            /* A poll snapshot cannot refer to a newly admitted slot incarnation. */
            short ev = captured[i].generation == s->handle.generation &&
                               f[i + 1].fd == s->fd
                           ? f[i + 1].revents
                           : 0;
            if (ev & (POLLERR | POLLNVAL))
            {
                release(&pool, s);
                continue;
            }
            if (s->flow.draining)
            {
                if (ev & (POLLIN | POLLHUP))
                {
                    unsigned char discard[32];
                    ssize_t n = recv(s->fd, discard, sizeof discard, 0);
                    if (n == 0 || (n < 0 && errno != EAGAIN &&
                                   errno != EWOULDBLOCK && errno != EINTR))
                    {
                        release(&pool, s);
                        continue;
                    }
                }
                if (hf_tick(&s->flow, now, d) == 2)
                    release(&pool, s);
                continue;
            }
            if ((ev & (POLLIN | POLLHUP)) && !s->flow.closing &&
                s->flow.count < cap && !s->have)
            {
                ssize_t n = recv(s->fd, s->input, sizeof s->input, 0);
                if (n > 0)
                {
                    s->have = (size_t)n;
                    s->used = 0;
                }
                else
                {
                    int event = n == 0                                    ? 0
                                : errno == EINTR                          ? -1
                                : errno == EAGAIN || errno == EWOULDBLOCK ? -2
                                                                          : -3;
                    if (hf_read_event(&s->flow, event, now, d))
                    {
                        release(&pool, s);
                        continue;
                    }
                }
            }
            if (s->have)
            {
                size_t used;
                if (hf_feed(&s->flow, s->input + s->used, s->have - s->used,
                            &used, now, authority, d))
                {
                    release(&pool, s);
                    continue;
                }
                s->used += used;
                if (s->used == s->have)
                    s->used = s->have = 0;
            }
            if ((ev & POLLOUT) &&
                hf_write(&s->flow, write_socket, &s->fd, now) < 0)
            {
                release(&pool, s);
                continue;
            }
            int action = hf_tick(&s->flow, now, d);
            if (action == 1)
                shutdown(s->fd, SHUT_WR);
            else if (action == 2 || action == 3)
                release(&pool, s);
        }
    }
    for (unsigned i = 0; i < 4; i++)
        release(&pool, &c[i]);
    close(listener);
    return 0;
}
