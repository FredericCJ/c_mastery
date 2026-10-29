#include "alu.h"
#include <limits.h>
#define CHECK(x)                      \
    do                                \
    {                                 \
        if (!(x))                     \
        {                             \
            aa_failure(#x, __LINE__); \
            return 41;                \
        }                             \
    } while (0)
_Static_assert(UINT_MAX == UINT32_MAX, "selected32-bit unsigned model");

static int equal(const struct aa_result *a, const struct aa_result *b)
{
    const unsigned char *x = (const unsigned char *)a,
                        *y = (const unsigned char *)b;
    for (unsigned i = 0; i < sizeof *a; ++i)
    {
        if (x[i] != y[i])
        {
            return 0;
        }
    }
    return 1;
}

static void bytes(struct aa_result *to, const struct aa_result *from)
{
    unsigned char *x = (unsigned char *)to;
    const unsigned char *y = (const unsigned char *)from;
    for (unsigned i = 0; i < sizeof *to; ++i)
    {
        x[i] = y[i];
    }
}

static int truth(void)
{
    for (unsigned a = 0; a < 2; ++a)
    {
        for (unsigned b = 0; b < 2; ++b)
        {
            for (unsigned c = 0; c < 2; ++c)
            {
                struct aa_slice r = {99, 88};
                CHECK(aa_slice(a, b, c, &r) == AA_OK);
                CHECK(r.sum == (a + b + c) % 2 && r.carry == (a + b + c) / 2);
                unsigned row[] = {a, b, c, r.sum, r.carry};
                aa_emit("SLICE", row, 5);
            }
        }
    }
    struct aa_slice r = {99, 88};
    CHECK(aa_slice(2, 0, 0, &r) == AA_INVALID && r.sum == 99 && r.carry == 88);
    CHECK(aa_slice(0, 2, 0, &r) == AA_INVALID &&
          aa_slice(0, 0, 2, &r) == AA_INVALID &&
          aa_slice(0, 0, 0, 0) == AA_INVALID);
    return 0;
}

static int exhaustive(void)
{
    for (unsigned width = 4; width <= 5; ++width)
    {
        unsigned size = 1u << width, half = size / 2;
        for (unsigned op = 0; op < 2; ++op)
        {
            for (unsigned input = 0; input < 2; ++input)
            {
                unsigned carry_count = 0, overflow_count = 0;
                for (unsigned a = 0; a < size; ++a)
                {
                    for (unsigned b = 0; b < size; ++b)
                    {
                        struct aa_result r;
                        CHECK(aa_alu(width, op, a, b, input, &r) == AA_OK);
                        int sa = a >= half ? (int)a - (int)size : (int)a,
                            sb = b >= half ? (int)b - (int)size : (int)b;
                        int mathematical = op ? (int)a - (int)b - (int)input
                                              : (int)a + (int)b + (int)input;
                        int signed_math =
                            op ? sa - sb - (int)input : sa + sb + (int)input;
                        unsigned value = (unsigned)mathematical & (size - 1);
                        unsigned overflow = signed_math < -(int)half ||
                                            signed_math >= (int)half;
                        unsigned borrow = mathematical < 0;
                        unsigned carm =
                                     op ? !borrow : mathematical >= (int)size,
                                 cf = op ? borrow : carm;
                        CHECK(r.value == value && r.z == (value == 0) &&
                              r.n == (value >= half));
                        CHECK(r.c_arm == carm && r.cf_x86 == cf &&
                              r.v == overflow);
                        CHECK(!(r.z && r.n));
                        carry_count += cf;
                        overflow_count += overflow;
                        unsigned reconstructed = 0;
                        for (unsigned i = 0; i < width; ++i)
                        {
                            CHECK(r.bit[i] < 2 && r.carry[i] < 2);
                            CHECK(((a >> i) & 1u) + (((b >> i) & 1u) ^ op) +
                                      r.carry[i] ==
                                  r.bit[i] + 2 * r.carry[i + 1]);
                            reconstructed |= r.bit[i] << i;
                        }
                        CHECK(reconstructed == r.value &&
                              r.carry[0] == (op ? 1 - input : input));
                        if (width == 4 && !op)
                        {
                            algo_alu_result old = algo_alu4(
                                (uint8_t)a, (uint8_t)b, (uint8_t)input);
                            CHECK(r.value == old.value &&
                                  r.c_arm == old.carry && r.v == old.overflow);
                        }
                        unsigned row[] = {width,   op,       input, a,
                                          b,       r.value,  r.z,   r.n,
                                          r.c_arm, r.cf_x86, r.v};
                        aa_emit("ALU", row, 11);
                    }
                }
                CHECK(carry_count ==
                          size *
                              (size + (input ? 1u : 0u) - (input ? 0u : 1u)) /
                              2 &&
                      overflow_count == size * size / 4);
                unsigned row[] = {width, op, input, carry_count,
                                  overflow_count};
                aa_emit("COUNTS", row, 5);
            }
        }
    }
    return 0;
}

static int admission(void)
{
    struct aa_result
        r = {77, 66, 55, 44, 33, 22, {11, 10, 9, 8, 7, 6}, {5, 4, 3, 2, 1}},
        old;
    bytes(&old, &r);
    const unsigned bad[][5] = {{0, 0, 0, 0, 0},  {3, 0, 0, 0, 0},
                               {6, 0, 0, 0, 0},  {32, 0, 0, 0, 0},
                               {4, 2, 0, 0, 0},  {4, UINT_MAX, 0, 0, 0},
                               {4, 0, 16, 0, 0}, {4, 0, 0, 16, 0},
                               {5, 0, 32, 0, 0}, {4, 0, 0, 0, 2}};
    for (unsigned i = 0; i < sizeof bad / sizeof bad[0]; ++i)
    {
        CHECK(aa_alu(bad[i][0], bad[i][1], bad[i][2], bad[i][3], bad[i][4],
                     &r) == AA_INVALID &&
              equal(&r, &old));
    }
    CHECK(aa_alu(4, 0, 0, 0, 0, 0) == AA_INVALID);
    CHECK(aa_alu(4, 0, 1, 2, 0, &r) == AA_OK && r.value == 3);
    return 0;
}

static int branches(void)
{
    unsigned divergent = 0;
    for (unsigned width = 4; width <= 5; ++width)
    {
        unsigned size = 1u << width, half = size / 2;
        for (unsigned a = 0; a < size; ++a)
        {
            for (unsigned b = 0; b < size; ++b)
            {
                struct aa_result r;
                CHECK(aa_alu(width, 1, a, b, 0, &r) == AA_OK);
                int sa = a >= half ? (int)a - (int)size : (int)a,
                    sb = b >= half ? (int)b - (int)size : (int)b;
                unsigned expected = (a < b) | ((a > b) << 1) |
                                    ((sa < sb) << 2) | ((sa > sb) << 3) |
                                    ((a == b) << 4);
                CHECK(aa_relations(&r) == expected);
                unsigned actual = aa_isa_compare((uint32_t)a << (32 - width),
                                                 (uint32_t)b << (32 - width));
#ifdef AA_ARM
                unsigned native_c = r.c_arm;
#else
                unsigned native_c = r.cf_x86;
#endif
                CHECK(actual ==
                      (expected | (native_c << 5) | (r.v << 6) | (r.n << 7)));
                divergent += (a < b) != (sa < sb);
                unsigned row[] = {width, a, b, expected, actual};
                aa_emit("BRANCH", row, 5);
            }
        }
    }
    CHECK(divergent == 640);
    return 0;
}

static int same(const char *a, const char *b)
{
    while (*a && *a == *b)
    {
        ++a;
        ++b;
    }
    return *a == *b;
}

int aa_test(const char *n)
{
    if (same(n, "truth"))
    {
        return truth();
    }
    if (same(n, "exhaustive"))
    {
        return exhaustive();
    }
    if (same(n, "admission"))
    {
        return admission();
    }
    if (same(n, "branches"))
    {
        return branches();
    }
    return 2;
}
