#include "free.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define CHECK(x)                                          \
    do                                                    \
    {                                                     \
        if (!(x))                                         \
        {                                                 \
            fprintf(stderr, "FREE-CHECK:%d\n", __LINE__); \
            return 1;                                     \
        }                                                 \
    } while (0)

static void seed(unsigned char a[24])
{
    for (unsigned i = 0; i < 24; ++i)
    {
        a[i] = (unsigned char)(31 * i + 17);
    }
}

static int same128(struct fr_u128 a, struct fr_u128 b)
{
    for (unsigned i = 0; i < 4; ++i)
    {
        if (a.limb[i] != b.limb[i])
        {
            return 0;
        }
    }
    return 1;
}

int main(void)
{
    unsigned triples = 0, copies = 0;
    for (size_t n = 0; n <= 16; ++n)
    {
        for (size_t s = 0; s + n <= 16; ++s)
        {
            for (size_t d = 0; d + n <= 16; ++d)
            {
                unsigned char a[24], before[24], expected[24];
                seed(a);
                memcpy(before, a, 24);
                memcpy(expected, a, 24);
                for (size_t i = 0; i < n; ++i)
                {
                    expected[4 + d + i] = before[4 + s + i];
                }
                CHECK(fr_move(a + 4 + d, a + 4 + s, n) == a + 4 + d);
                CHECK(memcmp(a, expected, 24) == 0);
                seed(a);
                CHECK(fr_arena_move(a + 4, 16, d, s, n) == FR_OK);
                CHECK(memcmp(a, expected, 24) == 0);
                int overlap = n != 0 && (d > s ? d - s < n : s - d < n);
                seed(a);
                enum fr_status st = fr_arena_copy(a + 4, 16, d, s, n);
                CHECK(st == (overlap ? FR_OVERLAP : FR_OK));
                CHECK(memcmp(a, overlap ? before : expected, 24) == 0);
                if (!overlap)
                {
                    seed(a);
                    CHECK(fr_copy(a + 4 + d, a + 4 + s, n) == a + 4 + d);
                    CHECK(memcmp(a, expected, 24) == 0);
                    ++copies;
                }
                seed(a);
                memcpy(expected, a, 24);
                for (size_t i = 0; i < n; ++i)
                {
                    expected[4 + d + i] = 0x80;
                }
                CHECK(fr_set(a + 4 + d, 0x80, n) == a + 4 + d);
                CHECK(memcmp(a, expected, 24) == 0);
                ++triples;
            }
        }
    }
    CHECK(triples == 1785 && copies > 0);
    for (unsigned a = 0; a < 256; ++a)
    {
        for (unsigned b = 0; b < 256; ++b)
        {
            unsigned char x = (unsigned char)a, y = (unsigned char)b;
            int cmp = fr_compare(&x, &y, 1), back = fr_compare(&y, &x, 1);
            CHECK((cmp > 0) - (cmp < 0) == (a > b) - (a < b));
            CHECK((cmp > 0) - (cmp < 0) == -((back > 0) - (back < 0)));
        }
    }
    for (size_t n = 0; n <= 16; ++n)
    {
        for (size_t split = 0; split <= n; ++split)
        {
            unsigned char source[16], one[16], two[16];
            for (unsigned i = 0; i < 16; ++i)
            {
                source[i] = (unsigned char)(7 * i + 128);
                one[i] = two[i] = 0;
            }
            fr_copy(one, source, n);
            fr_copy(two, source, split);
            fr_copy(two + split, source + split, n - split);
            CHECK(memcmp(one, two, 16) == 0);
        }
    }
    unsigned char a[16], before[16];
    for (unsigned i = 0; i < 16; ++i)
    {
        a[i] = before[i] = (unsigned char)i;
    }
    CHECK(fr_arena_move(a, 16, 17, 0, 0) == FR_RANGE);
    CHECK(fr_arena_move(a, 16, 0, 15, 2) == FR_RANGE);
    CHECK(fr_arena_copy(a, 16, SIZE_MAX, 0, 1) == FR_RANGE);
    CHECK(memcmp(a, before, 16) == 0);
    CHECK(fr_arena_move(0, 0, 0, 0, 0) == FR_OK);
    CHECK(fr_arena_copy(0, 0, 0, 0, 0) == FR_OK);
    CHECK(fr_arena_move(0, 1, 0, 0, 0) == FR_ARGUMENT);
    CHECK(fr_copy(0, 0, 0) == 0 && fr_move(0, 0, 0) == 0 &&
          fr_set(0, 0, 0) == 0 && fr_compare(0, 0, 0) == 0);
    unsigned char *x = malloc(16), *y = malloc(16);
    CHECK(x && y);
    for (unsigned i = 0; i < 16; ++i)
    {
        x[i] = (unsigned char)i;
    }
    fr_move(y, x, 16);
    CHECK(memcmp(x, y, 16) == 0);
    free(x);
    free(y);
    struct fr_u128 n = {{UINT32_MAX, UINT32_MAX, UINT32_MAX, UINT32_MAX}},
                   d = {{7, 0, 0, 0x80000000U}}, q = {{0, 0, 0, 0}},
                   r = {{0, 0, 0, 0}}, one = {{1, 0, 0, 0}},
                   expected = {
                       {0xfffffff8U, UINT32_MAX, UINT32_MAX, 0x7fffffffU}};
    CHECK(fr_divmod128(n, d, &q, &r) == FR_OK && same128(q, one) &&
          same128(r, expected));
    struct fr_u128 zero = {{0, 0, 0, 0}};
    q = n;
    r = n;
    CHECK(fr_divmod128(n, zero, &q, &r) == FR_DIVZERO && same128(q, n) &&
          same128(r, n));
    r = (struct fr_u128){{3, 0, 0, 0x80000000U}};
    unsigned bit = 9;
    expected = (struct fr_u128){{0, 0, 0, 0x80000000U}};
    CHECK(fr_divstep128(r, d, 1, &q, &bit) == FR_OK && bit == 1 &&
          same128(q, expected));
    q = n;
    bit = 9;
    CHECK(fr_divstep128(d, d, 0, &q, &bit) == FR_RANGE && same128(q, n) &&
          bit == 9);
    CHECK(fr_divstep128(r, d, 2, &q, &bit) == FR_ARGUMENT && same128(q, n) &&
          bit == 9);
    printf("FREE-CORE-PASS triples=%u copy=%u compares=65536 split=153\n",
           triples, copies);
    return 0;
}
