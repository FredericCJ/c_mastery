#include "checkpoint_channel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "CP_CHECK: %s:%d %s\n", __FILE__, __LINE__, #x); \
            exit(1);                                                         \
        }                                                                    \
    } while (0)

struct cpx_script
{
    const long *outcomes;
    size_t count, at, remaining[16], offset[16];
    const unsigned char *base;
    unsigned char sink[32];
    size_t used;
};

static struct cpw_result write_script(void *context, const unsigned char *data,
                                      size_t n)
{
    struct cpx_script *s = context;
    CHECK(s->at < s->count && s->at < 16);
    s->remaining[s->at] = n;
    s->offset[s->at] = s->base ? (size_t)(data - s->base) : 0;
    long raw = s->outcomes[s->at++];
    struct cpw_result r = cpw_raw(raw);
    if (r.kind == CPW_PROGRESS)
    {
        CHECK(r.accepted <= n && s->used + r.accepted <= sizeof s->sink);
        memcpy(s->sink + s->used, data, r.accepted);
        s->used += r.accepted;
    }
    return r;
}

void cp_test_writer(void)
{
    const unsigned char input[] = "ABCDEFGHI";
    const long steps[] = {3, -4, 2, 4};
    struct cpx_script s = {.outcomes = steps, .count = 4, .base = input};
    struct cpw_transfer t;
    CHECK(cpw_begin(&t, input, 9, 71) == CPW_PENDING);
    for (size_t i = 0; i < 4; ++i)
        CHECK(cpw_step(&t, write_script, &s) ==
              (i == 3 ? CPW_DONE : CPW_PENDING));
    const size_t remaining[] = {9, 6, 6, 4}, offset[] = {0, 3, 3, 5};
    for (size_t i = 0; i < 4; ++i)
        CHECK(s.remaining[i] == remaining[i] && s.offset[i] == offset[i]);
    CHECK(s.used == 9 && !memcmp(s.sink, input, 9) && t.accepted == 9 &&
          t.error.accepted == 9);
    CHECK(cpw_step(&t, write_script, &s) == CPW_DONE && s.at == 4);
    const long blocked[] = {3, 0, -11, 2, 4};
    s = (struct cpx_script){.outcomes = blocked, .count = 5, .base = input};
    CHECK(cpw_begin(&t, input, 9, 72) == CPW_PENDING);
    for (size_t i = 0; i < 5; ++i)
    {
        CHECK(cpw_step(&t, write_script, &s) ==
              (i == 4 ? CPW_DONE : CPW_PENDING));
        CHECK(s.at == i + 1);
    }
    CHECK(s.used == 9 && !memcmp(s.sink, input, 9));
    struct cpw_result raw = cpw_raw(-2), script = cpw_script(-2),
                      libc = cpw_libc(-1, 4);
    CHECK(raw.kind == CPW_ERROR && raw.cause == 2 && script.kind == CPW_RETRY &&
          script.cause == 1002 && libc.kind == CPW_RETRY && libc.cause == 4);
    CHECK(cpw_begin(&t, NULL, 0, 73) == CPW_DONE &&
          cpw_step(&t, write_script, &s) == CPW_DONE && s.at == 5);
}

void cp_test_writer_ownership(void)
{
    struct cpw_channel c;
    cpw_channel_init(&c);
    const int values[] = {65, 66};
    CHECK(cpw_offer(&c, values, 2) == CPM_OK);
    const long steps[] = {-11, 1, -5, 1};
    struct cpx_script s = {.outcomes = steps, .count = 4};
    CHECK(cpw_channel_step(&c, write_script, &s) == CPW_PENDING &&
          c.has_active && c.queue.count == 1 && s.used == 0);
    CHECK(cpw_channel_step(&c, write_script, &s) == CPW_DONE && !c.has_active &&
          c.queue.count == 1 && s.used == 1 && s.sink[0] == 65);
    CHECK(cpw_channel_step(&c, write_script, &s) == CPW_FAILED &&
          c.has_active && c.queue.count == 0 && s.used == 1);
    cpw_cleanup(&c.transfer, 9);
    cpw_cleanup(&c.transfer, 10);
    CHECK(c.transfer.error.primary == 5 && c.transfer.error.cleanup == 9 &&
          c.transfer.error.operation == 42 && c.transfer.error.accepted == 0);
    CHECK(cpw_channel_step(&c, write_script, &s) == CPW_FAILED && s.at == 3);
    CHECK(cpw_recover(&c.transfer) == CPW_PENDING &&
          c.transfer.error.primary == 5);
    CHECK(cpw_channel_step(&c, write_script, &s) == CPW_DONE && s.used == 2 &&
          s.sink[1] == 66 && !c.has_active);
    CHECK(cpw_channel_step(&c, write_script, &s) == CPW_DONE && s.at == 4);
    for (unsigned i = 0; i < CPM_SLOTS; ++i)
        CHECK(c.pool.state[i] == 0);
    const unsigned char span[] = "ABCDEFGHI";
    const long spans[] = {3, -5, 2, 4};
    s = (struct cpx_script){.outcomes = spans, .count = 4, .base = span};
    struct cpw_transfer t;
    CHECK(cpw_begin(&t, span, 9, 19) == CPW_PENDING);
    CHECK(cpw_step(&t, write_script, &s) == CPW_PENDING && t.accepted == 3);
    CHECK(cpw_step(&t, write_script, &s) == CPW_FAILED &&
          t.error.accepted == 3 && t.error.primary == 5);
    CHECK(cpw_recover(&t) == CPW_PENDING);
    CHECK(cpw_step(&t, write_script, &s) == CPW_PENDING &&
          cpw_step(&t, write_script, &s) == CPW_DONE);
    CHECK(s.used == 9 && !memcmp(s.sink, span, 9));
}
