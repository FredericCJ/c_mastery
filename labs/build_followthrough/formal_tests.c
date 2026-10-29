#include "formal.h"
#include <limits.h>
#include <stdio.h>
#define CHECK(c)                                      \
    do                                                \
    {                                                 \
        if (!(c))                                     \
        {                                             \
            printf("BF_CHECK:%d:%s\n", __LINE__, #c); \
            return 1;                                 \
        }                                             \
    } while (0)

struct script
{
    ptrdiff_t result[4];
    unsigned calls;
    size_t offset[4];
    const unsigned char *base;
};

static ptrdiff_t scripted(void *v, const unsigned char *p, size_t n)
{
    struct script *s = v;
    (void)n;
    s->offset[s->calls] = (size_t)(p - s->base);
    return s->result[s->calls++];
}

int main(void)
{
    unsigned a[4], out[2] = {99, 123}, runs = 0;
    for (size_t n = 0; n <= 4; ++n)
    {
        unsigned possibilities = 1;
        for (size_t i = 0; i < n; ++i)
            possibilities *= 11;
        for (unsigned code = 0; code < possibilities; ++code)
        {
            unsigned x = code, expected = 0;
            for (size_t i = 0; i < n; ++i)
            {
                a[i] = x % 11;
                x /= 11;
                expected += a[i];
            }
            out[0] = 99;
            CHECK(bf_sum(a, 4, n, out) == BF_OK);
            CHECK(out[0] == expected && out[1] == 123);
            CHECK(bf_sum_pairs(a, 4, n, out) == BF_OK && out[0] == expected);
            ++runs;
        }
    }
    a[0] = 3;
    a[1] = 4;
    a[2] = 5;
    CHECK(bf_sum(a, 4, 3, a + 1) == BF_OK && a[1] == 12 && a[0] == 3 &&
          a[2] == 5);
    out[0] = 99;
    a[0] = 11;
    CHECK(bf_sum(a, 4, 1, out) == BF_RANGE && out[0] == 99);
    CHECK(bf_sum(a, 0, 1, out) == BF_INVALID && out[0] == 99);
    CHECK(bf_sum(a, 4, 5, out) == BF_INVALID && out[0] == 99);
    CHECK(bf_sum(NULL, 0, 0, out) == BF_OK && out[0] == 0);
    CHECK(bf_sum(NULL, 1, 1, out) == BF_INVALID);
    out[0] = 99;
    CHECK(bf_max(a, 4, 0, out) == BF_EMPTY && out[0] == 99);
    a[0] = 2;
    a[1] = 9;
    a[2] = 4;
    CHECK(bf_max(a, 4, 3, out) == BF_OK && out[0] == 9);
    int signed_a[4] = {INT_MAX, 1, -1, 0}, signed_out = 77;
    CHECK(bf_signed(signed_a, 4, 3, &signed_out) == BF_RANGE &&
          signed_out == 77);
    signed_a[0] = INT_MIN;
    signed_a[1] = -1;
    CHECK(bf_signed(signed_a, 4, 2, &signed_out) == BF_RANGE &&
          signed_out == 77);
    signed_a[0] = 10;
    signed_a[1] = -10;
    CHECK(bf_signed(signed_a, 4, 2, &signed_out) == BF_OK && signed_out == 0);
    uint8_t b = 250, c = 10;
    CHECK(b + c == 260 && (uint8_t)(b + c) == 4);
    unsigned u = UINT_MAX;
    CHECK(u + 1u == 0);
    const unsigned char data[4] = {1, 2, 3, 4};
    size_t committed = 99;
    struct script s = {{1, -1, 0, 0}, 0, {0, 0, 0, 0}, data};
    CHECK(bf_write(scripted, &s, data, 4, 4, &committed) == BF_IO &&
          committed == 1 && s.calls == 2 && s.offset[1] == 1);
    s = (struct script){{0, 0, 0, 0}, 0, {0, 0, 0, 0}, data};
    CHECK(bf_write(scripted, &s, data, 4, 3, &committed) == BF_PENDING &&
          committed == 0 && s.calls == 3);
    s = (struct script){{5, 0, 0, 0}, 0, {0, 0, 0, 0}, data};
    CHECK(bf_write(scripted, &s, data, 4, 4, &committed) == BF_INVALID &&
          committed == 0 && s.calls == 1);
    s = (struct script){{1, 0, 3, 0}, 0, {0, 0, 0, 0}, data};
    CHECK(bf_write(scripted, &s, data, 4, 4, &committed) == BF_OK &&
          committed == 4 && s.offset[1] == 1 && s.offset[2] == 1);
    unsigned status = 3;
    CHECK(bf_read_clear(&status) == 3 && status == 2 &&
          bf_read_clear(&status) == 2);
    printf(
        "BF_PASS:formal:%u finite sequences; alias; signed range; frame; retries; read-clear\n",
        runs);
    return 0;
}
