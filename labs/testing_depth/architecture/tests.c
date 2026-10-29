#include "architecture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "TA_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

struct ta_script
{
    unsigned char bytes[8];
    size_t used, calls;
    unsigned mode;
};

static struct ta_write_result script(void *p, const unsigned char *bytes,
                                     size_t n)
{
    struct ta_script *s = p;
    s->calls++;
    if (s->mode == 1)
        return (struct ta_write_result){TA_WRITE_WAIT, 0};
    if (s->mode == 2)
        return (struct ta_write_result){TA_WRITE_OK, 0};
    if (s->mode == 3)
        return (struct ta_write_result){TA_WRITE_ERROR, 0};
    if (s->mode == 4)
        return (struct ta_write_result){TA_WRITE_OK, n + 1};
    if (s->mode == 5)
    {
        s->bytes[s->used++] = bytes[0];
        return (struct ta_write_result){TA_WRITE_ERROR, 1};
    }
    s->bytes[s->used++] = bytes[0];
    return (struct ta_write_result){TA_WRITE_OK, 1};
}

int main(void)
{
    for (unsigned layout = 0; layout < 4; layout++)
        for (unsigned cap = 1; cap <= 8; cap++)
        {
            struct
            {
                uint64_t before;
                struct ta_storage s;
                uint64_t after;
            } box = {0};
            struct ta_queue q = {0}, old;
            struct ta_storage saved;
            unsigned char out = 123, values[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
            size_t accepted = 99;
            box.before = UINT64_C(0x12345678);
            box.after = UINT64_C(0x87654321);
            old = q;
            CHECK(ta_init(&q, &box.s, (enum ta_layout)layout, cap, 0) ==
                  TA_INVALID);
            CHECK(memcmp(&q, &old, sizeof q) == 0);
            CHECK(ta_init(&q, &box.s, (enum ta_layout) - 1, cap, cap) ==
                  TA_INVALID);
            CHECK(ta_init(&q, &box.s, (enum ta_layout)layout, 0, 0) ==
                  TA_INVALID);
            CHECK(ta_init(&q, &box.s, (enum ta_layout)layout, 9, 9) ==
                  TA_INVALID);
            CHECK(ta_init(&q, &box.s, (enum ta_layout)layout, cap,
                          cap + (layout == TA_RING)) == TA_OK);
            CHECK(ta_invariant(&q));
            CHECK(ta_pop(&q, &out) == TA_EMPTY && out == 123);
            CHECK(ta_pop(&q, NULL) == TA_INVALID);
            for (unsigned round = 0; round < 5; round++)
            {
                CHECK(ta_batch(&q, values, cap + 1, &accepted) == TA_FULL &&
                      accepted == cap);
                CHECK(ta_count(&q) == cap);
                old = q;
                saved = box.s;
                CHECK(ta_push(&q, 99) == TA_FULL &&
                      memcmp(&old, &q, sizeof q) == 0 &&
                      memcmp(&saved, &box.s, sizeof saved) == 0);
                CHECK(ta_invariant(&q));
                for (unsigned i = 0; i < cap; i++)
                {
                    size_t count = ta_count(&q);
                    CHECK(ta_peek(&q, &out) == TA_OK && out == i &&
                          ta_count(&q) == count);
                    CHECK(ta_pop(&q, &out) == TA_OK && out == i);
                    CHECK(ta_invariant(&q));
                }
                CHECK(ta_count(&q) == 0);
            }
            CHECK(ta_batch(&q, NULL, 0, &accepted) == TA_OK && accepted == 0);
            CHECK(ta_batch(&q, NULL, 1, &accepted) == TA_INVALID &&
                  accepted == 0);
            CHECK(box.before == UINT64_C(0x12345678) &&
                  box.after == UINT64_C(0x87654321));
        }
    /* Harness allocation substitution: component receives identical typed objects. */
    for (unsigned mode = 0; mode < 2; mode++)
    {
        struct ta_storage local = {0}, *s = mode ? malloc(sizeof *s) : &local;
        struct ta_queue q;
        unsigned char out;
        CHECK(s);
        CHECK(ta_init(&q, s, TA_RING, 2, 3) == TA_OK);
        CHECK(ta_push(&q, 7) == TA_OK);
        CHECK(ta_push(&q, 8) == TA_OK);
        CHECK(ta_push(&q, 9) == TA_FULL);
        CHECK(ta_pop(&q, &out) == TA_OK && out == 7);
        CHECK(ta_pop(&q, &out) == TA_OK && out == 8);
        if (mode)
            free(s);
    }
    for (unsigned layout = 0; layout < 4; layout++)
    {
        unsigned char packet[3] = {2, 7, 8}, out[2] = {91, 92};
        struct ta_flow f = {0}, old;
        struct ta_script s = {0};
        CHECK(ta_decode(packet, 2, out) == TA_INVALID && out[0] == 91 &&
              out[1] == 92);
        old = f;
        CHECK(ta_begin(&f, packet, 2, (enum ta_layout)layout, 2) ==
                  TA_INVALID &&
              memcmp(&f, &old, sizeof f) == 0);
        CHECK(ta_begin(&f, packet, 3, (enum ta_layout)layout, 3) == TA_OK);
        packet[1] = 0;
        CHECK(ta_step(&f, script, &s) == TA_AGAIN);
        CHECK(ta_step(&f, script, &s) == TA_DONE);
        CHECK(s.used == 2 && s.bytes[0] == 7 && s.bytes[1] == 8);
        CHECK(ta_step(&f, script, &s) == TA_DONE && s.calls == 2);
        CHECK(ta_cancel(&f) == TA_DONE);
        for (unsigned mode = 1; mode <= 5; mode++)
        {
            s = (struct ta_script){0};
            s.mode = mode;
            CHECK(ta_begin(&f, packet, 3, (enum ta_layout)layout, 2) == TA_OK);
            if (mode <= 2)
            {
                CHECK(ta_step(&f, script, &s) == TA_AGAIN);
                CHECK(ta_step(&f, script, &s) == TA_TIMEOUT);
                CHECK(s.calls == 2);
            }
            else
                CHECK(ta_step(&f, script, &s) ==
                      (mode == 4 ? TA_INVALID : TA_IO));
            size_t calls = s.calls;
            CHECK(ta_step(&f, script, &s) == f.state && s.calls == calls);
            CHECK(f.uncertain == (mode == 4));
            if (mode == 5)
                CHECK(f.offset == 1 && s.used == 1);
        }
        s = (struct ta_script){0};
        CHECK(ta_begin(&f, packet, 3, (enum ta_layout)layout, 2) == TA_OK);
        old = f;
        CHECK(ta_step(&f, NULL, &s) == TA_INVALID &&
              memcmp(&old, &f, sizeof f) == 0);
        CHECK(ta_cancel(&f) == TA_CANCELLED);
        CHECK(ta_step(&f, script, &s) == TA_CANCELLED && s.calls == 0);
    }
    puts(
        "TA_PASS:four-layouts,sentinels,peek,batch,allocator,flow,budget,terminal");
    return 0;
}
