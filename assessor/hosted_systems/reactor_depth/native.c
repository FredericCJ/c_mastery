#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "reactor.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#define CHECK(x)                                                         \
    do                                                                   \
    {                                                                    \
        if (!(x))                                                        \
        {                                                                \
            fprintf(stderr, "REACTOR-NATIVE-CHECK:%s:%d errno=%d\n", #x, \
                    __LINE__, errno);                                    \
            return 1;                                                    \
        }                                                                \
    } while (0)

static int would_block(void)
{
    return errno == EAGAIN || errno == EWOULDBLOCK;
}

static int close_pair(int p[2])
{
    int a = close(p[0]), b = close(p[1]);
    return a || b;
}

static int cursor(void)
{
    int p[2];
    CHECK(!pipe2(p, O_NONBLOCK | O_CLOEXEC));
    unsigned char data[3] = {7, 0, 9}, buf[8];
    struct rd_operation op;
    CHECK(!rd_operation_init(&op, 3, 0) && write(p[1], data, 2) == 2);
    struct pollfd ready = {p[0], POLLIN, 0};
    CHECK(poll(&ready, 1, 0) == 1);
    ssize_t n = read(p[0], buf, 3);
    CHECK(n == 2 && rd_progress(&op, (int)n, buf) == 1 && op.done == 2);
    CHECK(read(p[0], buf, 1) == -1 && would_block() &&
          rd_progress(&op, -2, 0) == 0 && op.terminal == RD_ACTIVE);
    CHECK(write(p[1], data + 2, 1) == 1 && poll(&ready, 1, 0) == 1 &&
          read(p[0], buf, 1) == 1 && rd_progress(&op, 1, buf) == 1);
    CHECK(op.terminal == RD_DONE && !memcmp(op.bytes, data, 3) &&
          !close(p[1]) && read(p[0], buf, 1) == 0 && !close(p[0]));
    CHECK(!pipe2(p, O_NONBLOCK | O_CLOEXEC) && write(p[1], data, 2) == 2);
    ready = (struct pollfd){p[0], POLLIN, 0};
    CHECK(poll(&ready, 1, 0) == 1);
    int competing = dup(p[0]);
    CHECK(competing >= 0 && read(competing, buf, 2) == 2);
    CHECK(!rd_operation_init(&op, 3, 0) && read(p[0], buf, 3) == -1 &&
          would_block() && rd_progress(&op, -2, 0) == 0 && op.done == 0 &&
          op.terminal == RD_ACTIVE);
    CHECK(!close(competing) && !close_pair(p));
    puts("REACTOR-CURSOR:progress=2,again,1,eof;second_consumer_drained=2;"
         "immediate_again=1;");
    return 0;
}

static int prefill(int fd, size_t *count)
{
    unsigned char block[1024] = {0};
    *count = 0;
    while (*count < 65536)
    {
        ssize_t n = send(fd, block, sizeof block, MSG_NOSIGNAL);
        if (n > 0)
        {
            *count += (size_t)n;
            continue;
        }
        if (n == -1 && would_block())
            return 0;
        return -1;
    }
    return -1;
}

static int drain_exact(int fd, size_t count)
{
    unsigned char bytes[1024];
    while (count)
    {
        size_t want = count < sizeof bytes ? count : sizeof bytes;
        ssize_t n = recv(fd, bytes, want, 0);
        if (n <= 0)
            return -1;
        count -= (size_t)n;
    }
    return 0;
}

static int duplex(void)
{
    int p[2], size = 1024;
    CHECK(!socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0,
                      p) &&
          !setsockopt(p[0], SOL_SOCKET, SO_SNDBUF, &size, sizeof size));
    size_t retained;
    CHECK(!prefill(p[0], &retained));
    unsigned char input[3] = {5, 0, 7}, output[3] = {11, 12, 13}, b;
    struct rd_operation rx, tx;
    CHECK(!rd_operation_init(&rx, 3, 0) && !rd_operation_init(&tx, 3, output));
    CHECK(send(p[0], tx.bytes, tx.goal, MSG_NOSIGNAL) == -1 && would_block() &&
          rd_progress(&tx, -2, 0) == 0 && tx.done == 0);
    CHECK(!drain_exact(p[1], retained));
    for (unsigned i = 0; i < 3; ++i)
    {
        CHECK(send(p[1], input + i, 1, MSG_NOSIGNAL) == 1 &&
              recv(p[0], &b, 1, 0) == 1 && rd_progress(&rx, 1, &b) == 1);
        if (i < 2)
        {
            CHECK(send(p[0], tx.bytes + tx.done, 1, MSG_NOSIGNAL) == 1 &&
                  rd_progress(&tx, 1, 0) == 1 && recv(p[1], &b, 1, 0) == 1 &&
                  b == output[i]);
        }
    }
    CHECK(rx.terminal == RD_DONE && tx.terminal == RD_ACTIVE && tx.done == 2 &&
          !memcmp(rx.bytes, input, 3) && !memcmp(tx.bytes, output, 3));
    CHECK(rd_cancel(&tx) == 1 && rd_cancel(&tx) == 0 &&
          recv(p[1], &b, 1, 0) == -1 && would_block());
    CHECK(!close_pair(p));
    printf(
        "REACTOR-DUPLEX:rx=3;tx=2;tx_cancelled=1;prefill=%zu;again_preserved="
        "1;\n",
        retained);
    return 0;
}

static int notification(unsigned et, unsigned oneshot)
{
    int p[2], ep = epoll_create1(EPOLL_CLOEXEC);
    CHECK(ep >= 0 && !pipe2(p, O_NONBLOCK | O_CLOEXEC));
    struct epoll_event registration = {.events = EPOLLIN | (et ? EPOLLET : 0) |
                                                 (oneshot ? EPOLLONESHOT : 0),
                                       .data.u64 = 7},
                       event;
    CHECK(!epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &registration) &&
          write(p[1], "abcde", 5) == 5 && epoll_wait(ep, &event, 1, 0) == 1 &&
          event.data.u64 == 7);
    unsigned char bytes[6] = {0};
    CHECK(read(p[0], bytes, 2) == 2);
    int second = epoll_wait(ep, &event, 1, 0);
    CHECK(second == (et || oneshot ? 0 : 1));
    /* Explicit runnable carry after quota2; do not await another edge. */
    CHECK(read(p[0], bytes + 2, 2) == 2 && read(p[0], bytes + 4, 2) == 1 &&
          !memcmp(bytes, "abcde", 5));
    CHECK(read(p[0], bytes, 1) == -1 && would_block());
    if (oneshot)
    {
        CHECK(write(p[1], "Z", 1) == 1 && epoll_wait(ep, &event, 1, 0) == 0);
        CHECK(!epoll_ctl(ep, EPOLL_CTL_MOD, p[0], &registration) &&
              epoll_wait(ep, &event, 1, 0) == 1 && read(p[0], bytes, 1) == 1 &&
              bytes[0] == 'Z');
    }
    CHECK(!epoll_ctl(ep, EPOLL_CTL_DEL, p[0], 0) && !close_pair(p) &&
          !close(ep));
    printf(
        "REACTOR-EPOLL:et=%u;oneshot=%u;after_partial_notification=%d;carried_"
        "bytes=3;arrival_before_rearm=%u;\n",
        et, oneshot, second, oneshot);
    return 0;
}

static uint64_t token_value(struct rd_token t)
{
    return (uint64_t)t.generation * 4 + t.slot;
}

static struct rd_token token_decode(uint64_t v)
{
    return (struct rd_token){(unsigned)(v % 4), (unsigned)(v / 4)};
}

static int reuse(void)
{
    int p[2], q[2], ep = epoll_create1(EPOLL_CLOEXEC);
    CHECK(ep >= 0 && !pipe2(p, O_NONBLOCK | O_CLOEXEC) &&
          !pipe2(q, O_NONBLOCK | O_CLOEXEC));
    struct rd_registry registry;
    struct rd_token old, fresh;
    CHECK(!rd_registry_init(&registry, 7, 8) &&
          rd_register(&registry, 2, p[0], &old) == 1);
    struct epoll_event registration = {.events = EPOLLIN,
                                       .data.u64 = token_value(old)},
                       cached, current;
    unsigned char b;
    CHECK(!epoll_ctl(ep, EPOLL_CTL_ADD, p[0], &registration) &&
          write(p[1], "A", 1) == 1 && epoll_wait(ep, &cached, 1, 0) == 1);
    int number = p[0];
    CHECK(!epoll_ctl(ep, EPOLL_CTL_DEL, number, 0) &&
          rd_close(&registry, old) == 1 && !close(number));
    CHECK(dup2(q[0], number) == number && !close(q[0]) &&
          rd_register(&registry, 2, number, &fresh) == 1 &&
          fresh.generation == 8);
    registration.data.u64 = token_value(fresh);
    CHECK(!epoll_ctl(ep, EPOLL_CTL_ADD, number, &registration));
    /* Only stable registry metadata is consulted before payload access. Cached
     * event data never contains an unvalidated connection pointer. */
    CHECK(rd_borrow(&registry, token_decode(cached.data.u64)) == 0);
    CHECK(write(q[1], "B", 1) == 1 && epoll_wait(ep, &current, 1, 0) == 1);
    struct rd_slot *s = rd_borrow(&registry, token_decode(current.data.u64));
    CHECK(s && s->descriptor == number && read(s->descriptor, &b, 1) == 1 &&
          b == 'B');
    CHECK(!rd_return(&registry, fresh) &&
          !epoll_ctl(ep, EPOLL_CTL_DEL, number, 0) &&
          rd_close(&registry, fresh) == 1 && registry.slots[2].retired);
    CHECK(!close(number) && !close(p[1]) && !close(q[1]) && !close(ep));
    puts("REACTOR-REUSE:slot=2;old=7;new=8;same_descriptor=1;stale_rejected_"
         "before_payload=1;retired=1;");
    return 0;
}

static uint64_t now_ns(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t))
        abort();
    return (uint64_t)t.tv_sec * 1000000000u + (uint64_t)t.tv_nsec;
}

static int overload_policies(void)
{
    for (unsigned policy = 0; policy < 3; ++policy)
    {
        struct rd_service s;
        struct rf_message m[10];
        struct rf_frame frame = {.length = 1, .bytes = {42}};
        int pair[4][2], ep = epoll_create1(EPOLL_CLOEXEC);
        unsigned at;
        CHECK(ep >= 0 && !rd_service_init(&s, 256));
        for (unsigned i = 0; i < 4; ++i)
            CHECK(!socketpair(AF_UNIX,
                              SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0,
                              pair[i]) &&
                  rd_service_open(&s, &at) == 1 && at == i);
        struct epoll_event event = {.events = EPOLLIN, .data.u64 = 1}, observed;
        CHECK(!epoll_ctl(ep, EPOLL_CTL_ADD, pair[0][0], &event));
        for (unsigned i = 0; i < 9; ++i)
        {
            CHECK(!rf_message_init(&m[i], i + 1, 1, &frame) &&
                  rd_admit(&s, 0, &m[i], (enum rd_policy)policy) == (i < 8));
        }
        CHECK(m[8].owner == RF_CALLER);
        if (policy == RD_DISCONNECT)
        {
            CHECK(s.terminal == 8 && !s.outbox.connections[0].active &&
                  !epoll_ctl(ep, EPOLL_CTL_DEL, pair[0][0], 0) &&
                  !close(pair[0][0]));
            pair[0][0] = -1;
            unsigned char byte;
            CHECK(recv(pair[0][1], &byte, 1, 0) == 0);
        }
        else
        {
            if (policy == RD_PAUSE_READ)
            {
                event.events = 0;
                CHECK(s.paused == 1 &&
                      !epoll_ctl(ep, EPOLL_CTL_MOD, pair[0][0], &event));
            }
            const unsigned char incoming[2] = {1, 42};
            unsigned char received[2];
            CHECK(send(pair[0][1], incoming, 2, MSG_NOSIGNAL) == 2);
            CHECK(epoll_wait(ep, &observed, 1, 0) ==
                  (policy == RD_PAUSE_READ ? 0 : 1));
            if (policy == RD_PAUSE_READ)
            {
                for (unsigned i = 0; i < 5; ++i)
                {
                    const unsigned char *wire;
                    size_t n;
                    CHECK(rf_peek(&s.outbox, 0, &wire, &n) == 1 && n == 2 &&
                          send(pair[0][0], wire, n, MSG_NOSIGNAL) == 2 &&
                          !rd_commit(&s, 0, 2) &&
                          recv(pair[0][1], received, 2, 0) == 2 &&
                          !memcmp(received, incoming, 2));
                }
                CHECK(rd_resume(&s, 0) == 1);
                event.events = EPOLLIN;
                CHECK(!epoll_ctl(ep, EPOLL_CTL_MOD, pair[0][0], &event) &&
                      epoll_wait(ep, &observed, 1, 0) == 1);
            }
            CHECK(recv(pair[0][0], received, 2, 0) == 2);
            struct rf_parser parser;
            struct rf_frame parsed;
            size_t consumed;
            CHECK(!rf_parser_init(&parser, 1) &&
                  rf_feed(&parser, received, 2, &consumed, &parsed) == 1 &&
                  consumed == 2 && parsed.length == 1 && parsed.bytes[0] == 42);
            CHECK(!rf_message_init(&m[9], 10, 1, &parsed) &&
                  rd_admit(&s, 0, &m[9], (enum rd_policy)policy) ==
                      (policy == RD_PAUSE_READ));
            CHECK(!epoll_ctl(ep, EPOLL_CTL_DEL, pair[0][0], 0));
        }
        CHECK(!rd_stop(&s, 0, 0) && s.phase == RD_RELEASED &&
              s.terminal == s.accepted);
        for (unsigned i = 0; i < 4; ++i)
        {
            if (pair[i][0] >= 0)
                CHECK(!close(pair[i][0]));
            CHECK(!close(pair[i][1]));
        }
        CHECK(!close(ep));
    }
    puts("REACTOR-POLICIES:actual_drop=1;actual_disconnect=1;pause_interest_"
         "suppressed=1;resumed_frame=42;");
    return 0;
}

static int shutdown_service(void)
{
    struct rd_service s;
    struct rf_message m[32], rejected;
    struct rf_frame frame = {.length = 8, .bytes = {1, 2, 3, 4, 5, 6, 7, 8}};
    int sockets[4][2], size = 1024;
    size_t pre[4];
    unsigned at;
    CHECK(!rd_service_init(&s, 256));
    for (unsigned i = 0; i < 4; ++i)
    {
        CHECK(!socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                          0, sockets[i]) &&
              !setsockopt(sockets[i][0], SOL_SOCKET, SO_SNDBUF, &size,
                          sizeof size) &&
              !prefill(sockets[i][0], &pre[i]) &&
              rd_service_open(&s, &at) == 1 && at == i);
    }
    at = 99;
    CHECK(rd_service_open(&s, &at) == 0 && at == 99);
    for (unsigned i = 0; i < 32; ++i)
    {
        CHECK(!rf_message_init(&m[i], i + 1, 2, &frame) &&
              rd_admit(&s, i / 8, &m[i], RD_DROP_NEWEST) == 1);
    }
    CHECK(!rf_message_init(&rejected, 33, 2, &frame) &&
          rd_admit(&s, 0, &rejected, RD_DROP_NEWEST) == 0 &&
          rejected.owner == RF_CALLER);
    for (unsigned i = 0; i < 4; ++i)
    {
        const unsigned char *wire;
        size_t n;
        CHECK(rf_peek(&s.outbox, i, &wire, &n) == 1 &&
              send(sockets[i][0], wire, n, MSG_NOSIGNAL) == -1 &&
              would_block());
    }
    uint64_t begin = now_ns(), deadline = begin + 20000000u;
    CHECK(!rd_service_borrow(&s) && !rd_stop(&s, begin, deadline) &&
          s.phase == RD_DRAINING);
    CHECK(!drain_exact(sockets[0][1], pre[0]));
    const unsigned char *wire;
    size_t n;
    CHECK(rf_peek(&s.outbox, 0, &wire, &n) == 1 && n == 10);
    ssize_t sent = send(sockets[0][0], wire, n, MSG_NOSIGNAL);
    CHECK(sent == 10 && !rd_commit(&s, 0, (size_t)sent) &&
          !drain_exact(sockets[0][1], 10) && s.terminal == 1);
    for (;;)
    {
        uint64_t observed = now_ns();
        if (observed >= deadline)
            break;
        uint64_t remaining = deadline - observed;
        struct timespec t = {(time_t)(remaining / 1000000000u),
                             (long)(remaining % 1000000000u)};
        int r = ppoll(0, 0, &t, 0);
        CHECK(r == 0 || (r == -1 && errno == EINTR));
    }
    uint64_t finish = now_ns();
    CHECK(!rd_tick(&s, finish) && s.phase == RD_CANCELLING && s.terminal == 1 &&
          s.borrowers == 1);
    for (unsigned i = 1; i < 32; ++i)
        CHECK(m[i].owner == RF_OUTBOX);
    CHECK(!rd_service_return(&s) && s.phase == RD_RELEASED &&
          !rd_stop(&s, finish, finish) && s.terminal == 32);
    for (unsigned i = 0; i < 32; ++i)
        CHECK(s.records[i].id == i + 1 &&
              s.records[i].outcome == (i ? RF_CANCELLED : RF_DELIVERED));
    for (unsigned i = 0; i < 4; ++i)
        CHECK(!close_pair(sockets[i]));
    printf("REACTOR-SHUTDOWN:accepted=32;completed=1;cancelled=31;deferred_"
           "borrow=1;elapsed_ns=%llu;\n",
           (unsigned long long)(finish - begin));
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "cursor"))
        return cursor();
    if (!strcmp(argv[1], "duplex"))
        return duplex();
    if (!strcmp(argv[1], "lt"))
        return notification(0, 0);
    if (!strcmp(argv[1], "et"))
        return notification(1, 0);
    if (!strcmp(argv[1], "oneshot"))
        return notification(1, 1);
    if (!strcmp(argv[1], "reuse"))
        return reuse();
    if (!strcmp(argv[1], "shutdown"))
        return shutdown_service();
    if (!strcmp(argv[1], "policies"))
        return overload_policies();
    return 2;
}
