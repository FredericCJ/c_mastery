#define _GNU_SOURCE
#include "stream.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <termios.h>
#include <unistd.h>
#define CHECK(x)                                                               \
    do                                                                         \
    {                                                                          \
        if (!(x))                                                              \
        {                                                                      \
            fprintf(stderr, "PL_CHECK:%d:%s errno=%d\n", __LINE__, #x, errno); \
            exit(1);                                                           \
        }                                                                      \
    } while (0)
/* Generated independently with Python zlib.crc32; campaign recomputes both. */
static const unsigned char first[] = {165, 1,  52, 18,  3,   0,  65,
                                      0,   10, 31, 137, 151, 109};
static const unsigned char second[] = {165, 1,   120, 86, 2,  0,
                                       165, 255, 167, 14, 66, 63};

struct pt_transport
{
    int tx, rx;
};

struct pt_observed
{
    struct cp_message messages[8];
    size_t count;
};

static struct pt_transport transport(unsigned kind)
{
    struct pt_transport t = {-1, -1};
    if (kind == 1)
    {
        char name[128];
        t.tx = posix_openpt(O_RDWR | O_NOCTTY | O_CLOEXEC);
        CHECK(t.tx >= 0 && grantpt(t.tx) == 0 && unlockpt(t.tx) == 0 &&
              ptsname_r(t.tx, name, sizeof name) == 0);
        t.rx = open(name, O_RDWR | O_NOCTTY | O_CLOEXEC);
        CHECK(t.rx >= 0);
        struct termios a;
        CHECK(tcgetattr(t.rx, &a) == 0);
        cfmakeraw(&a);
        a.c_cc[VMIN] = 1;
        a.c_cc[VTIME] = 0;
        CHECK(tcsetattr(t.rx, TCSANOW, &a) == 0);
    }
    else if (kind == 2)
    {
        int pair[2];
        CHECK(socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, pair) == 0);
        t.tx = pair[0];
        t.rx = pair[1];
    }
    return t;
}

static void finish(struct pt_transport t)
{
    if (t.rx >= 0)
    {
        CHECK(close(t.rx) == 0);
    }
    if (t.tx >= 0)
    {
        CHECK(close(t.tx) == 0);
    }
}

static void take_one(struct pt_session *s, struct pt_observed *o)
{
    CHECK(o->count < 8 && pt_session_take(s, &o->messages[o->count]) == CP_OK);
    ++o->count;
}

static void consume(struct pt_session *s, const unsigned char *b, size_t n,
                    struct pt_observed *o)
{
    size_t done = 0;
    unsigned turns = 0;
    for (;;)
    {
        size_t used = SIZE_MAX;
        CHECK(++turns < 256);
        enum cp_status r = pt_session_feed(s, b + done, n - done, &used);
        CHECK(used <= n - done);
        done += used;
        if (r == CP_FULL)
        {
            take_one(s, o);
            continue;
        }
        CHECK(r == CP_NEED_MORE && done == n);
        break;
    }
}

static void send_bytes(struct pt_transport t, struct pt_session *s,
                       const unsigned char *b, size_t n, size_t chunk,
                       struct pt_observed *o)
{
    if (t.tx >= 0)
    {
        size_t sent = 0;
        while (sent < n)
        {
            ssize_t count = write(t.tx, b + sent, n - sent);
            CHECK(count > 0);
            sent += (size_t)count;
        }
    }
    size_t done = 0;
    while (done < n)
    {
        size_t want = n - done;
        if (want > chunk)
        {
            want = chunk;
        }
        unsigned char buffer[128];
        CHECK(want <= sizeof buffer);
        if (t.rx < 0)
        {
            consume(s, b + done, want, o);
            done += want;
        }
        else
        {
            struct pollfd p = {t.rx, POLLIN, 0};
            CHECK(poll(&p, 1, 2000) == 1);
            ssize_t count = read(t.rx, buffer, want);
            CHECK(count > 0);
            consume(s, buffer, (size_t)count, o);
            done += (size_t)count;
        }
    }
}

static void drain(struct pt_session *s, struct pt_observed *o)
{
    while (s->queue.count)
    {
        take_one(s, o);
    }
}

static void message(const struct cp_message *m, unsigned sequence,
                    const unsigned char *b, size_t n)
{
    CHECK(m->sequence == sequence && m->length == n &&
          memcmp(m->payload, b, n) == 0);
}

static void corpus(unsigned kind, size_t chunk)
{
    const unsigned char prefix[] = {0, 0xa5, 2, 0xa5, 1, 0, 0, 255, 255};
    unsigned char input[128];
    size_t n = 0;
    memcpy(input, prefix, sizeof prefix);
    n += sizeof prefix;
    memcpy(input + n, first, sizeof first);
    input[n + sizeof first - 1] ^= 1;
    n += sizeof first;
    memcpy(input + n, second, sizeof second);
    n += sizeof second;
    memcpy(input + n, first, sizeof first);
    n += sizeof first;
    memcpy(input + n, second, sizeof second);
    n += sizeof second;
    struct pt_session s;
    struct pt_observed o = {0};
    pt_session_init(&s);
    CHECK(pt_session_open(&s) == CP_OK);
    struct pt_transport t = transport(kind);
    send_bytes(t, &s, input, n, chunk, &o);
    drain(&s, &o);
    CHECK(o.count == 3 && s.rejects > 0);
    message(&o.messages[0], 0x5678, second + 6, 2);
    message(&o.messages[1], 0x1234, first + 6, 3);
    message(&o.messages[2], 0x5678, second + 6, 2);
    pt_session_close(&s);
    finish(t);
}

static void reconnect(unsigned kind)
{
    struct pt_session s;
    struct pt_observed o = {0};
    pt_session_init(&s);
    CHECK(pt_session_open(&s) == CP_OK && s.epoch == 1);
    struct pt_transport t = transport(kind);
    send_bytes(t, &s, first, 8, 3, &o);
    CHECK(o.count == 0 && s.parser.used == 8);
    pt_session_close(&s);
    finish(t);
    size_t used = 123;
    CHECK(pt_session_feed(&s, first, 1, &used) == CP_INVALID && used == 123);
    CHECK(pt_session_open(&s) == CP_OK && s.epoch == 2);
    t = transport(kind);
    send_bytes(t, &s, first + 8, sizeof first - 8, 2, &o);
    send_bytes(t, &s, second, sizeof second, 7, &o);
    drain(&s, &o);
    CHECK(o.count == 1);
    message(&o.messages[0], 0x5678, second + 6, 2);
    send_bytes(t, &s, first, sizeof first, 13, &o);
    CHECK(s.queue.count == 1);
    pt_session_close(&s);
    CHECK(pt_session_open(&s) == CP_OK && s.queue.count == 0 &&
          s.parser.used == 0);
    pt_session_close(&s);
    finish(t);
    s.epoch = UINT64_MAX;
    CHECK(pt_session_open(&s) == CP_INVALID && !s.connected &&
          s.epoch == UINT64_MAX);
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    if (!strcmp(argv[1], "segmentation"))
    {
        for (unsigned kind = 0; kind < 3; ++kind)
        {
            for (size_t n = 1; n <= 59; ++n)
            {
                corpus(kind, n);
            }
        }
        puts(
            "PS_TRANSFER adapters=3 chunk_bounds=59 same_parser_object independent_crc_vectors=2");
    }
    else if (!strcmp(argv[1], "reconnect"))
    {
        for (unsigned kind = 0; kind < 3; ++kind)
        {
            reconnect(kind);
        }
        puts("PS_RECONNECT adapters=3 partial_and_queued_reset=1");
    }
    else
    {
        CHECK(0);
    }
    puts("PS_PASS");
    return 0;
}
