#define _GNU_SOURCE
#include "service.h"
#include "bridge.h"
#include "coroutines.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#define CHECK(x)                                                  \
    do                                                            \
    {                                                             \
        if (!(x))                                                 \
        {                                                         \
            fprintf(stderr, "CAP_COMPOSE:%s:%d\n", #x, __LINE__); \
            return 1;                                             \
        }                                                         \
    } while (0)

static struct timespec deadline(unsigned seconds)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t))
    {
        abort();
    }
    t.tv_sec += seconds;
    return t;
}

static int p5_case(int cancel, unsigned split)
{
    int fds[2];
    CHECK(!pipe2(fds, O_NONBLOCK | O_CLOEXEC));
    struct p5_bridge p;
    CHECK(!p5_bridge_init(&p, fds[0], 1));
    CHECK(!close(fds[0]));
    struct cs_service s;
    CHECK(cs_init(&s, 1, 10) == CS_OK);
    struct cp_message m = {73, 32, {0}};
    for (unsigned i = 0; i < 32; ++i)
    {
        m.payload[i] = (unsigned char)(0xa5u + i);
    }
    CHECK(cs_offer(&s, &m, 1, 0, 0) == CS_FRAME && cs_begin(&s) == CS_OK);
    unsigned char received[42];
    size_t total = 0;
    unsigned turns = 0;
    while (s.has_active)
    {
        const unsigned char *bytes;
        size_t n;
        unsigned generation;
        CHECK(cs_lease(&s, &bytes, &n, &generation) == CS_OK);
        size_t part = n < split ? n : split;
        CHECK(!p5_bridge_offer(&p, (int32_t)m.sequence));
        uint32_t ticket;
        CHECK(!p5_bridge_begin(&p, &ticket));
        CHECK(write(fds[1], bytes, part) == (ssize_t)part);
        struct timespec until = deadline(3);
        CHECK(!pthread_mutex_lock(&p.mutex));
        while (p.used < part)
        {
            CHECK(!pthread_cond_timedwait(&p.changed, &p.mutex, &until));
        }
        CHECK(!memcmp(p.bytes, bytes, part));
        CHECK(!pthread_mutex_unlock(&p.mutex));
        CHECK(p5_bridge_cancel(&p, ticket) == 1 &&
              p5_bridge_observe(&p, ticket) == 1);
        if (cancel)
        {
            CHECK(cs_close(&s, 1) == CS_PENDING && s.has_active &&
                  s.releases == 0);
        }
        CHECK(cs_complete(&s, generation, 0, 0, NULL, NULL) == CS_PENDING &&
              s.borrowed && s.offset == total);
        struct timespec now = deadline(0);
        CHECK(p5_bridge_wait(&p, &now) == 1 && p5_bridge_destroy(&p) == 1);
        CHECK(!p5_bridge_release_gate(&p));
        until = deadline(3);
        CHECK(!p5_bridge_wait(&p, &until) && !p5_bridge_observe(&p, ticket));
        CHECK(!p5_bridge_reap(&p));
        memcpy(received + total, p.bytes, part);
        total += part;
        enum cs_status st = cs_complete(&s, generation, 1, part, NULL, NULL);
        CHECK(st == (cancel ? CS_CLOSED : total == 42 ? CS_DONE : CS_PENDING));
        CHECK(cs_complete(&s, generation, 1, 0, NULL, NULL) == CS_STALE);
        ++turns;
    }
    CHECK(p.joins == turns && p.operation.releases == turns &&
          p.discarded == turns);
    unsigned char expected[42];
    size_t n;
    CHECK(cp_encode(&m, expected, 42, &n) == CP_OK &&
          !memcmp(received, expected, total));
    CHECK(cs_conserved(&s) && s.releases == 1 &&
          s.delivered == (cancel ? 0u : 1u) &&
          s.discarded == (cancel ? 1u : 0u));
    CHECK(!p5_bridge_destroy(&p) && !close(fds[1]));
    printf("actual-P5 split=%u cancel=%d prefix=%zu joins=%u core-releases=1\n",
           split, cancel, total, turns);
    return 0;
}

struct cf_port
{
    const unsigned char *bytes;
    size_t position, length;
    unsigned calls, armed, disarms;
    int again;
};

static struct co_read_result read_port(void *p, unsigned char *out, size_t n)
{
    struct cf_port *s = p;
    ++s->calls;
    if (s->again)
    {
        s->again = 0;
        return (struct co_read_result){CO_READ_AGAIN, 0};
    }
    size_t k = s->length - s->position;
    if (k > n)
    {
        k = n;
    }
    if (k > 3)
    {
        k = 3;
    }
    for (size_t i = 0; i < k; ++i)
    {
        out[i] = s->bytes[s->position++];
    }
    return (struct co_read_result){k ? CO_READ_DATA : CO_READ_EOF, k};
}

static int arm(void *p, uint32_t generation)
{
    struct cf_port *s = p;
    if (s->armed || !generation)
    {
        return -1;
    }
    s->armed = 1;
    return 0;
}

static void disarm(void *p, uint32_t generation)
{
    struct cf_port *s = p;
    if (!generation || !s->armed)
    {
        abort();
    }
    s->armed = 0;
    ++s->disarms;
}

static int coroutine(void)
{
    struct co_async oversized = {0};
    CHECK(co_async_init(&oversized, 42, 1) == CO_INVALID);
    struct cp_message m = {91, 32, {0}};
    for (unsigned i = 0; i < 32; ++i)
    {
        m.payload[i] = (unsigned char)(i == 7 ? 0xa5 : i);
    }
    unsigned char wire[42];
    size_t n;
    CHECK(cp_encode(&m, wire, 42, &n) == CP_OK);
    struct cf_port p = {.bytes = wire, .length = 42};
    struct co_async_port port = {&p, read_port, arm, disarm};
    struct cs_service s;
    CHECK(cs_init(&s, 1, 10) == CS_OK);
    unsigned chunks = 0;
    while (p.position < p.length)
    {
        size_t wanted = p.length - p.position;
        if (wanted > 8)
        {
            wanted = 8;
        }
        struct co_async a;
        CHECK(co_async_init(&a, wanted, ++chunks) == CO_OK);
        p.again = 1;
        unsigned char out[8];
        size_t used = 0;
        int status = CO_PENDING;
        for (unsigned turn = 0; turn < 10 && status == CO_PENDING; ++turn)
        {
            status = co_async_poll(&a, port, out, &used);
            if (status == CO_PENDING && a.registered)
            {
                CHECK(co_async_notify(&a, a.generation) == CO_OK);
            }
        }
        CHECK(status == CO_DONE && used == wanted && !p.armed);
        size_t consumed = 0;
        enum cs_status accepted = cs_feed(&s, 0, out, used, &consumed, 1, 0);
        CHECK(consumed == used &&
              accepted == (p.position == 42 ? CS_FRAME : CS_MORE));
    }
    CHECK(chunks == 6 && p.disarms == 6 && s.accepted == 1 && cs_conserved(&s));
    struct co_async pending;
    CHECK(co_async_init(&pending, 8, 99) == CO_OK);
    p.position = 0;
    p.again = 1;
    unsigned char out[8];
    size_t got = 0;
    CHECK(co_async_poll(&pending, port, out, &got) == CO_PENDING && p.armed);
    CHECK(co_async_cancel(&pending, port) == CO_OK && !p.armed &&
          co_async_notify(&pending, 99) == CO_STATE);
    printf(
        "actual-coro42=rejected chunked=8,8,8,8,8,2 calls=%u synchronous-disarm-only\n",
        p.calls);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "p5"))
    {
        for (unsigned split = 13; split <= 32; split += 19)
        {
            for (int cancel = 0; cancel < 2; ++cancel)
            {
                if (p5_case(cancel, split))
                {
                    return 1;
                }
            }
        }
        return 0;
    }
    if (!strcmp(argv[1], "coroutine"))
    {
        return coroutine();
    }
    return 2;
}
