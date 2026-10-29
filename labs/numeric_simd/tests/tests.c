#include "core.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                              \
    do                                                                        \
    {                                                                         \
        if (!(x))                                                             \
        {                                                                     \
            fprintf(stderr, "B06_CHECK %s:%d: %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                         \
        }                                                                     \
    } while (0)

static int test_q15(void)
{
    CHECK(ns_q15(-32768, -32768) == 32767);
    CHECK(ns_q15(16384, 1) == 1);
    CHECK(ns_q15(-16384, 1) == -1);
    CHECK(ns_q15(0, 32767) == 0);
    CHECK(ns_q15(32767, 32767) == 32766);
    return 0;
}

static int test_rounding(void)
{
    int o = 99;
    CHECK(ns_round_even(5, 2, &o) && o == 2);
    CHECK(ns_round_even(7, 2, &o) && o == 4);
    CHECK(ns_round_even(-5, 2, &o) && o == -2);
    CHECK(ns_round_even(0, 1, &o) && o == 0);
    o = 99;
    CHECK(!ns_round_even(1, 0, &o) && o == 99);
    return 0;
}

static int test_dot(void)
{
    int16_t a[3] = {-32768, -32768, -32768};
    int64_t o = 7;
    CHECK(ns_dot(a, a, 3, &o) && o == INT64_C(3221225472));
    CHECK(ns_dot(NULL, NULL, 0, &o) && o == 0);
    o = 9;
    CHECK(!ns_dot(NULL, NULL, 65536, &o) && o == 9);
    return 0;
}

static int test_compensated(void)
{
    double a[3] = {1e16, 1, -1e16}, o = 99, b[1] = {INFINITY};
    CHECK(ns_sum(a, 3, &o) && o == 1);
    CHECK(ns_sum(NULL, 0, &o) && o == 0);
    o = 99;
    CHECK(!ns_sum(b, 1, &o) && o == 99);
    return 0;
}

static int test_norm(void)
{
    double a[2] = {3e200, 4e200}, o = 0, z[2] = {0, -0.0};
    CHECK(ns_norm(a, 2, &o) && fabs(o / 5e200 - 1) < 1e-14);
    CHECK(ns_norm(z, 2, &o) && o == 0);
    CHECK(ns_norm(NULL, 0, &o) && o == 0);
    return 0;
}

static int test_matrix(void)
{
    int32_t a[4] = {1, 2, 3, 4}, b[4] = {5, 6, 7, 8};
    int64_t o[4] = {0};
    CHECK(ns_matmul(a, b, 2, 2, 2, o));
    CHECK(o[0] == 19 && o[1] == 22 && o[2] == 43 && o[3] == 50);
    a[3] = 1025;
    o[0] = 99;
    CHECK(!ns_matmul(a, b, 2, 2, 2, o) && o[0] == 99);
    return 0;
}

static int test_lanes(void)
{
    int16_t a[5] = {32767, -32768, 1, 2, 3}, b[5] = {1, -1, 2, 3, 4},
            o[5] = {0};
    CHECK(ns_add(a, b, 5, o));
    CHECK(o[0] == 32767 && o[1] == -32768 && o[2] == 3 && o[3] == 5 &&
          o[4] == 7);
    CHECK(ns_add(NULL, NULL, 0, NULL));
    CHECK(!ns_add(NULL, NULL, 65536, NULL));
    return 0;
}

static int test_tolerance(void)
{
    CHECK(ns_close(1e-12, 0, 1e-11, 0));
    CHECK(!ns_close(1e-6, 0, 1e-11, 0));
    CHECK(ns_close(1.001, 1, 0, 0.002));
    CHECK(!ns_close(NAN, 1, 1, 1));
    CHECK(!ns_close(INFINITY, INFINITY, 0, 0));
    CHECK(!ns_close(1, 1, -1, 0));
    CHECK(!ns_close(1e308, -1e308, 1e308, 0));
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (strcmp(argv[1], "q15") == 0)
        return test_q15();
    if (strcmp(argv[1], "rounding") == 0)
        return test_rounding();
    if (strcmp(argv[1], "dot") == 0)
        return test_dot();
    if (strcmp(argv[1], "compensated") == 0)
        return test_compensated();
    if (strcmp(argv[1], "norm") == 0)
        return test_norm();
    if (strcmp(argv[1], "matrix") == 0)
        return test_matrix();
    if (strcmp(argv[1], "lanes") == 0)
        return test_lanes();
    if (strcmp(argv[1], "tolerance") == 0)
        return test_tolerance();
    return 2;
}
