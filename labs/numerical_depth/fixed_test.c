#include "test.h"

enum
{
    Q7 = 7,
    Q11 = 11,
    Q7_MIN = -128,
    Q7_MAX = 127,
    Q11_MIN = -2048,
    Q11_MAX = 2047
};

/*
 * Reference product in Q(q): round to nearest with ties away from zero, then
 * saturate to [-2^q, 2^q - 1]. C division truncates toward zero, so the
 * remainder has the sign of the product; the two corrections handle
 * positive and negative ties symmetrically without shifting negative values.
 */
static int reference_qmul(int a, int b, unsigned q)
{
    int unit = 1 << q;
    int product = a * b;
    int quotient = product / unit;
    int remainder = product % unit;
    if (remainder >= unit / 2)
    {
        quotient++;
    }
    if (remainder <= -unit / 2)
    {
        quotient--;
    }
    if (quotient >= unit)
    {
        quotient = unit - 1;
    }
    if (quotient < -unit)
    {
        quotient = -unit;
    }
    return quotient;
}

static void check_q7_exhaustive(void)
{
    int16_t out;
    for (int a = Q7_MIN; a <= Q7_MAX; a++)
    {
        for (int b = Q7_MIN; b <= Q7_MAX; b++)
        {
            CHECK(nd_qmul((int16_t)a, (int16_t)b, Q7, &out) == ND_OK);
            CHECK(out == reference_qmul(a, b, Q7));
        }
    }
}

/* Every 17th by 19th Q11 operand: a reproducible lattice, not exhaustive. */
static void check_q11_lattice(void)
{
    int16_t out;
    for (int a = Q11_MIN; a <= Q11_MAX; a += 17)
    {
        for (int b = Q11_MIN; b <= Q11_MAX; b += 19)
        {
            CHECK(nd_qmul((int16_t)a, (int16_t)b, Q11, &out) == ND_OK &&
                  out == reference_qmul(a, b, Q11));
        }
    }
}

/* The accumulator is rounded once, after all products are summed. */
static void check_multiply_accumulate(void)
{
    int16_t a[ND_QMAC_MAX];
    int16_t b[ND_QMAC_MAX];
    int16_t out;
    for (size_t i = 0; i < ND_QMAC_MAX; i++)
    {
        a[i] = Q7_MIN;
        b[i] = Q7_MIN;
    }
    CHECK(nd_qmac(a, b, ND_QMAC_MAX, Q7, &out) == ND_OK && out == Q7_MAX);
    for (size_t i = 0; i < ND_QMAC_MAX; i++)
    {
        a[i] = Q11_MIN;
        b[i] = Q11_MAX;
    }
    CHECK(nd_qmac(a, b, ND_QMAC_MAX, Q11, &out) == ND_OK && out == Q11_MIN);

    /* Two products of exactly half a unit each: compares one final rounding
     * (nd_qmac) with the sum of individually rounded products (nd_qmul). */
    a[0] = 1;
    a[1] = 1;
    b[0] = 64;
    b[1] = 64;
    CHECK(nd_qmac(a, b, 2, Q7, &out) == ND_OK && out == 1);
    int16_t single;
    CHECK(nd_qmul(1, 64, Q7, &single) == ND_OK && single + single == 2);
}

static void check_quantize(void)
{
    const double values[] = {-2,        -1, -.5,         -1.0 / 256, 0,
                             1.0 / 256, .5, 127.0 / 128, 1,          2};
    const int expected[] = {-128, -128, -64, -1, 0, 1, 64, 127, 127, 127};
    int16_t out;
    for (size_t i = 0; i < sizeof values / sizeof values[0]; i++)
    {
        CHECK(nd_quantize(values[i], Q7, &out) == ND_OK && out == expected[i]);
    }
}

static void check_rejections_keep_output(void)
{
    int16_t a[ND_QMAC_MAX + 1] = {0};
    int16_t b[ND_QMAC_MAX + 1] = {0};
    int16_t out = 42;
    CHECK(nd_quantize(NAN, Q7, &out) == ND_BAD && out == 42);
    CHECK(nd_qmul(128, 0, Q7, &out) == ND_BAD && out == 42);
    CHECK(nd_qmac(a, b, ND_QMAC_MAX + 1, Q7, &out) == ND_BAD && out == 42);
}

int main(void)
{
    check_q7_exhaustive();
    check_q11_lattice();
    check_multiply_accumulate();
    check_quantize();
    check_rejections_keep_output();
    puts("PASS exhaustive Q7, Q11 lattice, final-round MAC and clipping");
}
