#include "bringup.h"
#include <stdio.h>
#include <string.h>
#define CK(x)                                                  \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "BU_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

static int hook(void *p)
{
    unsigned *v = p;
    *v = *v * 10 + 1;
    return 0;
}

static int fail(void *p)
{
    ++*(unsigned *)p;
    return 1;
}

int main(void)
{
    const struct bu_range code = {0x08000000, 0x200000},
                          stack = {0x2002f000, 0x1000};
    unsigned char vec[8] = {0, 0, 3, 0x20, 1, 2, 0, 8};
    struct bu_vectors v = {0, 0};
    CK(bu_vectors(vec, 8, 1, code.base, code, stack, &v) == BU_OK &&
       v.stack == 0x20030000 && v.reset == 0x08000201);
    for (size_t n = 0; n < 8; n++)
    {
        CK(bu_vectors(vec, n, 1, code.base, code, stack, &v) == BU_INVALID);
    }
    CK(bu_vectors(vec, 8, 2, code.base, code, stack, &v) == BU_UNSUPPORTED);
    vec[4] = 0;
    CK(bu_vectors(vec, 8, 1, code.base, code, stack, &v) == BU_INVALID);
    vec[4] = 1;
    vec[0] = 4;
    CK(bu_vectors(vec, 8, 1, code.base, code, stack, &v) == BU_INVALID);
    vec[0] = 0;
    CK(bu_vectors(vec, 8, 1, code.base + 4, code, stack, &v) == BU_INVALID);
    CK(!bu_contains((struct bu_range){UINT32_MAX - 1, 4},
                    (struct bu_range){UINT32_MAX, 1}));
    CK(bu_contains((struct bu_range){9, 4}, (struct bu_range){13, 0}));
    CK(!bu_overlap((struct bu_range){9, 4}, (struct bu_range){13, 2}));
    unsigned char ram[64], rom[32], before[64];
    for (unsigned i = 0; i < 32; i++)
    {
        rom[i] = (unsigned char)(17 * i + 3);
    }
    memset(ram, 0xa5, sizeof ram);
    struct bu_copy cp[3] = {{1, 2, 7}, {13, 32, 8}, {32, 64, 0}};
    struct bu_range zp[2] = {{13, 5}, {45, 7}};
    CK(bu_memory(ram, 64, rom, 32, cp, 3, zp, 2, (struct bu_range){24, 8}) ==
       BU_OK);
    for (unsigned i = 0; i < 64; i++)
    {
        unsigned e = 0xa5;
        if (i >= 2 && i < 9)
        {
            e = rom[i - 1];
        }
        else if (i >= 32 && i < 40)
        {
            e = rom[i - 19];
        }
        else if ((i >= 13 && i < 18) || (i >= 45 && i < 52))
        {
            e = 0;
        }
        CK(ram[i] == e);
    }
    memcpy(before, ram, 64);
    cp[1].run = 4;
    CK(bu_memory(ram, 64, rom, 32, cp, 3, zp, 2, (struct bu_range){24, 8}) ==
           BU_INVALID &&
       memcmp(before, ram, 64) == 0);
    CK(bu_f439_dma((struct bu_range){0x2002fffc, 4}));
    CK(!bu_f439_dma((struct bu_range){0x2002fffc, 5}));
    CK(!bu_f439_dma((struct bu_range){0x10000000, 4}));
    struct bu_clock clock;
    for (unsigned limit = 1; limit <= 8; limit++)
    {
        for (unsigned ready = 1; ready <= limit + 1; ready++)
        {
            CK(bu_clock_start(&clock, limit, 1) == BU_OK);
            for (unsigned i = 1; i <= limit && clock.phase == 1; i++)
            {
                enum bu_result r = bu_clock_step(&clock, i == ready, 0);
                CK(r == (i == limit && ready > limit ? BU_TIMEOUT : BU_OK));
            }
            if (ready <= limit)
            {
                CK(clock.phase == 2 && !clock.dependencies);
                CK(bu_clock_reconfigure(&clock, 0) == BU_FAILED);
                for (unsigned i = 1; i <= limit; i++)
                {
                    CK(bu_clock_step(&clock, 1, i == limit) == BU_OK);
                }
                CK(clock.phase == 3 && clock.dependencies);
                CK(bu_clock_failure(&clock) == BU_OK && !clock.dependencies &&
                   clock.epoch == 1);
            }
            else
            {
                CK(clock.phase == 4 && !clock.dependencies);
            }
        }
    }
    unsigned calls = 0, done = 99;
    struct bu_hook h[3] = {
        {0, 1, hook, &calls}, {1, 2, hook, &calls}, {2, 0, hook, &calls}};
    CK(bu_hooks(0, 0, 0, &done) == BU_OK && !done);
    CK(bu_hooks(h, 3, 3, &done) == BU_OK && calls == 111 && done == 7);
    calls = 0;
    h[0].dependencies = 4;
    CK(bu_hooks(h, 3, 3, &done) == BU_INVALID && !calls && !done);
    h[0].dependencies = 0;
    CK(bu_hooks(h, 3, 1, &done) == BU_UNSUPPORTED && !calls);
    h[1].call = fail;
    CK(bu_hooks(h, 3, 3, &done) == BU_FAILED && done == 1 && calls == 2);
    unsigned char bytes[2][BU_RECORD];
    struct bu_record a = {1, 4, 255, {3, 4, 5, 6}},
                     b = {2, 8, 0, {9, 8, 7, 6, 5, 4, 3, 2}}, out = {0};
    unsigned writes, chosen;
    for (unsigned cut = 0; cut <= 25; cut++)
    {
        memset(bytes, 0, sizeof bytes);
        CK(bu_store(bytes[0], BU_RECORD, &a, 25, &writes) == BU_OK &&
           writes == 25);
        CK(bu_store(bytes[1], BU_RECORD, &b, cut, &writes) ==
               (cut == 25 ? BU_OK : BU_FAILED) &&
           writes == cut);
        CK(bu_select((const unsigned char (*)[BU_RECORD])bytes, &chosen) ==
               BU_OK &&
           chosen == (cut == 25 ? 1u : 0u));
    }
    bytes[1][12] ^= 1;
    CK(bu_decode(bytes[1], BU_RECORD, &out) == BU_INVALID);
    bytes[1][12] ^= 1;
    CK(bu_decode(bytes[1], BU_RECORD, &out) == BU_OK && out.payload[7] == 2);
    bytes[1][5] = 255;
    CK(bu_decode(bytes[1], BU_RECORD, &out) == BU_INVALID);
    CK(bu_migrate(&a, &out) == BU_OK && out.version == 2 && out.length == 8 &&
       out.payload[3] == 6 && out.payload[4] == 0);
    b.generation = 127;
    CK(bu_store(bytes[1], BU_RECORD, &b, 25, &writes) == BU_OK);
    CK(bu_select((const unsigned char (*)[BU_RECORD])bytes, &chosen) ==
       BU_AMBIGUOUS);
    struct bu_handoff ho = {0x20030000, 0x08000200, 0x08000401, 1, 1, 1,
                            1,          0,          1,          1, 1};
    CK(bu_handoff(&ho, code, stack) == BU_OK);
    ho.dma_active = 1;
    CK(bu_handoff(&ho, code, stack) == BU_INVALID);
    ho.dma_active = 0;
    ho.pending_clear = 0;
    CK(bu_handoff(&ho, code, stack) == BU_INVALID);
    puts("BU_PASS vectors memory clock hooks retention handoff");
    return 0;
}
