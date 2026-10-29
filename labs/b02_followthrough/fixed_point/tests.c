#include "fixed_point.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "BF_CHECK:%d:%s\n", __LINE__, #x); \
            exit(1);                                           \
        }                                                      \
    } while (0)

static void q7(void)
{
    int8_t a[1024], b[1024], out = 77;
    CHECK(b2_q7(NULL, NULL, 0, &out) == B2_OK && out == 0);
    for (unsigned i = 0; i < 1024; ++i)
    {
        a[i] = -128;
        b[i] = -128;
    }
    CHECK(b2_q7(a, b, 1024, &out) == B2_OK && out == 127);
    for (unsigned i = 0; i < 1024; ++i)
    {
        a[i] = i < 512 ? 127 : -127;
        b[i] = 127;
    }
    CHECK(b2_q7(a, b, 1024, &out) == B2_OK && out == 0);
    out = 77;
    CHECK(b2_q7(a, b, 1025, &out) == B2_RANGE && out == 77);
    const int8_t x[] = {1, 1}, y[] = {64, 64};
    CHECK(b2_q7(x, y, 2, &out) == B2_OK && out == 1 &&
          bf_q7_even(x, y, 2, &out) == B2_OK && out == 1);
    puts(
        "BF q7 validated actual-B02 full1024 extremes cancellation once-rounding admission");
}

static void even(void)
{
    int8_t values[] = {1, 3, 5, -1, -3, -5}, gain = 64, out = 77;
    int expected[] = {0, 2, 2, 0, -2, -2};
    for (unsigned i = 0; i < 6; ++i)
    {
        CHECK(bf_q7_even(&values[i], &gain, 1, &out) == B2_OK &&
              out == expected[i]);
    }
    CHECK(bf_q7_even(NULL, NULL, 0, &out) == B2_OK && out == 0);
    out = 77;
    CHECK(bf_q7_even(&values[0], &gain, 1025, &out) == B2_RANGE && out == 77 &&
          bf_q7_even(NULL, &gain, 1, &out) == B2_INVALID && out == 77);
    puts("BF even validated both-sign ties parity empty unchanged-refusal");
}

static void q11(void)
{
    static int16_t a[65536], b[65536];
    int16_t out = 77;
    for (unsigned i = 0; i < 65536; ++i)
    {
        a[i] = -32768;
        b[i] = -32768;
    }
    CHECK(bf_q11(a, b, 65536, &out) == B2_OK && out == 32767);
    for (unsigned i = 0; i < 65536; ++i)
    {
        a[i] = i < 32768 ? 32767 : -32767;
        b[i] = 32767;
    }
    CHECK(bf_q11(a, b, 65536, &out) == B2_OK && out == 0);
    for (unsigned i = 0; i < 8; ++i)
    {
        a[i] = -32768;
        b[i] = i < 4 ? -32768 : 32767;
    }
    CHECK(bf_q11(a, b, 8, &out) == B2_OK && out == 64);
    int16_t one = 1, half = 1024;
    CHECK(bf_q11(&one, &half, 1, &out) == B2_OK && out == 1);
    one = -1;
    CHECK(bf_q11(&one, &half, 1, &out) == B2_OK && out == -1 &&
          bf_q11(NULL, NULL, 0, &out) == B2_OK && out == 0);
    out = 77;
    CHECK(bf_q11(a, b, 65537, &out) == B2_RANGE && out == 77);
    puts(
        "BF q11 validated full65536 sum2power46 cancelled-wide-prefix half-ties scale2048");
}

static void units(void)
{
    struct bf_signal s;
    int8_t samples[] = {1, 1}, gains[] = {64, 64};
    CHECK(bf_signal(samples, gains, 2, &s) == B2_OK && s.product_sum == 128 &&
          s.rounded_raw == 1 && s.output == 1 && !s.clipped &&
          s.error_numerator == 0);
    int8_t min = -128;
    CHECK(bf_signal(&min, &min, 1, &s) == B2_OK && s.product_sum == 16384 &&
          s.rounded_raw == 128 && s.output == 127 && s.clipped &&
          s.error_numerator == -128);
    int8_t half = 64, one = 1;
    CHECK(bf_signal(&one, &half, 1, &s) == B2_OK && s.error_numerator == 64 &&
          !s.clipped);
    one = -1;
    CHECK(bf_signal(&one, &half, 1, &s) == B2_OK && s.error_numerator == -64 &&
          !s.clipped);
    int8_t a[] = {127, 127, -128}, b[] = {127, 127, 127};
    CHECK(bf_signal(a, b, 3, &s) == B2_OK && s.output == 125 &&
          s.product_sum == 16002 && s.error_numerator == -2);
    struct bf_signal prior = s;
    CHECK(bf_signal(a, b, 1025, &s) == B2_RANGE &&
          s.product_sum == prior.product_sum &&
          s.error_numerator == prior.error_numerator);
    puts(
        "BF units validated volts128 gains128 product16384 output128 clipping-breaks-half-LSB-bound");
}

static int oracle(void)
{
    unsigned n;
    while (scanf("%u", &n) == 1)
    {
        CHECK(n <= 1024);
        int8_t a[1024], b[1024];
        for (unsigned i = 0; i < n; ++i)
        {
            int x, y;
            CHECK(scanf("%d %d", &x, &y) == 2 && x >= -128 && x <= 127 &&
                  y >= -128 && y <= 127);
            a[i] = (int8_t)x;
            b[i] = (int8_t)y;
        }
        int8_t away = 77, even_value = 77;
        struct bf_signal s;
        CHECK(b2_q7(a, b, n, &away) == B2_OK &&
              bf_q7_even(a, b, n, &even_value) == B2_OK &&
              bf_signal(a, b, n, &s) == B2_OK);
        printf("%d %d %d %d %d %u\n", away, even_value, s.product_sum,
               s.rounded_raw, s.error_numerator, s.clipped);
    }
    return 0;
}

static int wide(void)
{
    unsigned n;
    static int16_t a[65536], b[65536];
    while (scanf("%u", &n) == 1)
    {
        CHECK(n <= 65536);
        for (unsigned i = 0; i < n; ++i)
        {
            int x, y;
            CHECK(scanf("%d %d", &x, &y) == 2 && x >= -32768 && x <= 32767 &&
                  y >= -32768 && y <= 32767);
            a[i] = (int16_t)x;
            b[i] = (int16_t)y;
        }
        int16_t out = 77;
        CHECK(bf_q11(a, b, n, &out) == B2_OK);
        printf("%d\n", out);
    }
    return 0;
}

static void scale16(void)
{
    struct bf_small s;
    int8_t a[] = {3, -3, 64, 0}, b[] = {8, 8, 64, 127};
    int expected[] = {2, -2, 127, 0};
    for (unsigned i = 0; i < 4; ++i)
    {
        CHECK(bf_mul16(a[i], b[i], BF_AWAY, &s) == B2_OK &&
              s.output == expected[i] && s.clipped == (i == 2));
    }
    int8_t ties[] = {1, 3, 5, -1, -3, -5};
    int even_result[] = {0, 2, 2, 0, -2, -2};
    for (unsigned i = 0; i < 6; ++i)
    {
        CHECK(bf_mul16(ties[i], 8, BF_EVEN, &s) == B2_OK &&
              s.output == even_result[i]);
    }
    struct bf_small prior = s;
    CHECK(bf_mul16(1, 1, 2, &s) == B2_INVALID &&
          s.error_numerator == prior.error_numerator);
    puts(
        "BF scale16 validated range-minus8-to127over16 example2-minus2-127-0 ties-even");
}

static void mixed(void)
{
    struct bf_small s;
    CHECK(bf_mix16_32(16, 32, BF_AWAY, &s) == B2_OK && s.output == 32 &&
          s.raw_numerator == 64 && s.raw_denominator == 2 && !s.clipped);
    CHECK(bf_mix16_32(0, 1, BF_AWAY, &s) == B2_OK && s.output == 1 &&
          bf_mix16_32(0, 1, BF_EVEN, &s) == B2_OK && s.output == 0);
    CHECK(bf_mix16_32(127, 127, BF_AWAY, &s) == B2_OK && s.output == 127 &&
          s.clipped);
    puts(
        "BF mixed validated volts16-plus-volts32 converted-before-add output16");
}

static void cascade(void)
{
    struct bf_cascade c;
    CHECK(bf_two16(3, 8, 127, BF_AWAY, &c) == B2_OK && c.first.output == 2 &&
          c.second.output == 16 && c.error_numerator == 1048 &&
          c.bound_numerator == 1144 && c.bounded);
    CHECK(bf_two16(64, 64, 16, BF_AWAY, &c) == B2_OK && !c.bounded &&
          c.first.clipped);
    struct bf_cascade prior = c;
    CHECK(bf_two16(1, 1, 1, 2, &c) == B2_INVALID &&
          c.error_numerator == prior.error_numerator);
    puts(
        "BF cascade validated propagated-bound-operand-magnitude clipping-separate");
}

static void show_small(struct bf_small s)
{
    printf(" %d %d %d %d %d %u", s.raw_numerator, s.raw_denominator,
           s.rounded_raw, s.error_numerator, s.output, s.clipped);
}

static int small_oracle(void)
{
    int a, b;
    while (scanf("%d %d", &a, &b) == 2)
    {
        CHECK(a >= -128 && a <= 127 && b >= -128 && b <= 127);
        for (unsigned mode = 0; mode < 2; ++mode)
        {
            struct bf_small x, y;
            CHECK(bf_mul16((int8_t)a, (int8_t)b, mode, &x) == B2_OK &&
                  bf_mix16_32((int8_t)a, (int8_t)b, mode, &y) == B2_OK);
            show_small(x);
            show_small(y);
        }
        putchar('\n');
    }
    return 0;
}

static int cascade_oracle(void)
{
    int a, b, c;
    unsigned mode;
    while (scanf("%d %d %d %u", &a, &b, &c, &mode) == 4)
    {
        CHECK(a >= -128 && a <= 127 && b >= -128 && b <= 127 && c >= -128 &&
              c <= 127 && mode < 2);
        struct bf_cascade out;
        CHECK(bf_two16((int8_t)a, (int8_t)b, (int8_t)c, mode, &out) == B2_OK);
        show_small(out.first);
        show_small(out.second);
        printf(" %d %u %u\n", out.error_numerator, out.bound_numerator,
               out.bounded);
    }
    return 0;
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    if (!strcmp(argv[1], "scale16"))
    {
        scale16();
    }
    else if (!strcmp(argv[1], "mixed"))
    {
        mixed();
    }
    else if (!strcmp(argv[1], "cascade"))
    {
        cascade();
    }
    else if (!strcmp(argv[1], "small_oracle"))
    {
        return small_oracle();
    }
    else if (!strcmp(argv[1], "cascade_oracle"))
    {
        return cascade_oracle();
    }
    else if (!strcmp(argv[1], "q7"))
    {
        q7();
    }
    else if (!strcmp(argv[1], "even"))
    {
        even();
    }
    else if (!strcmp(argv[1], "q11"))
    {
        q11();
    }
    else if (!strcmp(argv[1], "units"))
    {
        units();
    }
    else if (!strcmp(argv[1], "oracle"))
    {
        return oracle();
    }
    else if (!strcmp(argv[1], "wide"))
    {
        return wide();
    }
    else if (!strcmp(argv[1], "metrics"))
    {
        printf(
            "{\"int_bits\":%zu,\"size_bits\":%zu,\"q7_terms\":1024,\"q11_terms\":65536,\"q7_sum_bound\":16777216,\"q11_sum_bound\":70368744177664,\"signal_bytes\":%zu}\n",
            sizeof(int) * CHAR_BIT, sizeof(size_t) * CHAR_BIT,
            sizeof(struct bf_signal));
    }
    else
    {
        return 2;
    }
    return 0;
}
