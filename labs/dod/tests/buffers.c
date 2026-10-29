#include "dod_buffers.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const char *test_name = "arguments";
#define CHECK(x)                                                              \
    do                                                                        \
    {                                                                         \
        if (!(x))                                                             \
        {                                                                     \
            fprintf(stderr, "DOD_CHECK:%s:%d:%s\n", test_name, __LINE__, #x); \
            exit(1);                                                          \
        }                                                                     \
    } while (0)

static void buffers(void)
{
    unsigned char input[128];
    for (unsigned i = 0; i < 128; ++i)
    {
        input[i] = (unsigned char)i;
    }
    struct d_inline f = {0};
    CHECK(d_inline_append(&f, input, 16) == D_OK && f.length == 16);
    CHECK(d_inline_append(&f, input, 1) == D_FULL && f.length == 16);
    CHECK(d_inline_truncate(&f, 4) == D_OK &&
          d_inline_append(&f, input, 12) == D_OK);
    CHECK(d_inline_append(&f, NULL, 0) == D_OK);
    for (unsigned cut = 0; cut <= 2; ++cut)
    {
        struct d_quota q = {0};
        struct d_zero a, b, moved;
        d_zero_init(&a, &q);
        d_zero_init(&b, &q);
        d_zero_init(&moved, &q);
        CHECK(d_zero_append(&a, input, 16) == D_OK &&
              d_zero_append(&b, input, 16) == D_OK && q.live == 32);
        unsigned char *old_a = a.data, *old_b = b.data;
        q.fail_at = cut ? q.attempts + cut : 0;
        int result = d_zero_pair_append(&a, input + 16, 16, &b, input + 16, 16);
        if (cut)
        {
            CHECK(result == D_FAILED && a.data == old_a && b.data == old_b &&
                  a.length == 16 && b.length == 16 && q.live == 32);
            for (unsigned i = 0; i < 16; ++i)
            {
                CHECK(a.data[i] == i && b.data[i] == i);
            }
        }
        else
        {
            CHECK(result == D_OK && a.length == 32 && b.length == 32 &&
                  q.live == 64 && q.peak == 96);
            for (unsigned i = 0; i < 32; ++i)
            {
                CHECK(a.data[i] == i && b.data[i] == i);
            }
        }
        CHECK(d_zero_move(&moved, &a) == D_OK && !a.data && !a.length);
        d_zero_reset(&a);
        d_zero_reset(&moved);
        d_zero_reset(&b);
        CHECK(!q.live);
        for (unsigned i = 0; i < 4; ++i)
        {
            CHECK(!q.used[i]);
        }
    }
    struct d_quota q = {0};
    struct d_zero a, b;
    d_zero_init(&a, &q);
    d_zero_init(&b, &q);
    CHECK(d_zero_append(&a, input, 64) == D_OK);
    unsigned char *old = a.data;
    CHECK(d_zero_append(&a, input, 64) == D_FAILED && a.data == old &&
          a.length == 64 && q.live == 64);
    CHECK(d_zero_append(&b, input, 64) == D_OK && q.live == 128);
    CHECK(d_zero_pair_append(&a, NULL, 0, &b, NULL, 0) == D_OK &&
          q.live == 128);
    CHECK(d_zero_move(&a, &b) == D_INVALID);
    d_zero_reset(&a);
    d_zero_reset(&b);
    CHECK(q.live == 0);
    q.attempts = SIZE_MAX;
    CHECK(d_zero_append(&a, input, 1) == D_FAILED && q.attempts == SIZE_MAX &&
          !a.data);
    q.attempts = 0;
    q.releases = SIZE_MAX;
    CHECK(d_zero_append(&a, input, 1) == D_OK);
    d_zero_reset(&a);
    CHECK(!q.live && q.releases == SIZE_MAX);
    size_t attempts = q.attempts;
    CHECK(d_zero_pair_append(&a, input, 1, &b, input, 129) == D_FULL &&
          q.attempts == attempts && !a.data && !b.data);
}

static int32_t signed16(uint32_t x)
{
    return x < 32768 ? (int32_t)x : -1 - (int32_t)(65535 - x);
}

static uint32_t oracle(uint32_t a, uint32_t b)
{
    uint32_t r = 0;
    for (unsigned lane = 0; lane < 2; ++lane)
    {
        int32_t n = signed16((a >> (16 * lane)) & 65535) +
                    signed16((b >> (16 * lane)) & 65535);
        if (n > 32767)
        {
            n = 32767;
        }
        if (n < -32768)
        {
            n = -32768;
        }
        r |= ((uint32_t)n & 65535) << (16 * lane);
    }
    return r;
}

static void signed_lanes(void)
{
    const uint32_t edge[] = {0, 1, 32766, 32767, 32768, 32769, 65535};
    for (unsigned i = 0; i < 7; ++i)
    {
        for (unsigned j = 0; j < 7; ++j)
        {
            for (unsigned k = 0; k < 7; ++k)
            {
                for (unsigned l = 0; l < 7; ++l)
                {
                    uint32_t a = edge[i] | (edge[k] << 16),
                             b = edge[j] | (edge[l] << 16);
                    CHECK(d_swar16_signed(a, b) == oracle(a, b));
                }
            }
        }
    }
    uint32_t random = 1;
    for (unsigned i = 0; i < 100000; ++i)
    {
        random = random * 1664525u + 1013904223u;
        uint32_t a = random;
        random = random * 1664525u + 1013904223u;
        uint32_t b = random;
        CHECK(d_swar16_signed(a, b) == oracle(a, b));
    }
    CHECK(d_swar16_signed(UINT32_C(0x0000ffff), 1) == 0);
}

static void traces(void)
{
    const uint32_t edge[] = {0, 1, 32766, 32767, 32768, 32769, 65535};
    for (unsigned i = 0; i < 7; ++i)
    {
        for (unsigned j = 0; j < 7; ++j)
        {
            for (unsigned k = 0; k < 7; ++k)
            {
                for (unsigned l = 0; l < 7; ++l)
                {
                    uint32_t a = edge[i] | (edge[k] << 16),
                             b = edge[j] | (edge[l] << 16);
                    printf("S %u %u %u\n", a, b, d_swar16_signed(a, b));
                }
            }
        }
    }
    unsigned char bytes[32];
    for (unsigned i = 0; i < 32; ++i)
    {
        bytes[i] = (unsigned char)i;
    }
    for (unsigned cut = 0; cut <= 2; ++cut)
    {
        struct d_quota q = {0};
        struct d_zero a, b;
        d_zero_init(&a, &q);
        d_zero_init(&b, &q);
        CHECK(d_zero_append(&a, bytes, 16) == D_OK &&
              d_zero_append(&b, bytes, 16) == D_OK);
        q.fail_at = cut ? q.attempts + cut : 0;
        int s = d_zero_pair_append(&a, bytes + 16, 16, &b, bytes + 16, 16);
        printf("B %u %d %zu %zu %zu %zu %zu ", cut, s, a.length, b.length,
               q.live, q.peak, q.attempts);
        d_zero_reset(&a);
        d_zero_reset(&b);
        printf("%zu %zu\n", q.live, q.releases);
    }
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    test_name = argv[1];
    if (!strcmp(argv[1], "buffers"))
    {
        buffers();
    }
    else if (!strcmp(argv[1], "swarsigned"))
    {
        signed_lanes();
    }
    else if (!strcmp(argv[1], "oracle"))
    {
        traces();
    }
    else
    {
        CHECK(0);
    }
    return 0;
}
