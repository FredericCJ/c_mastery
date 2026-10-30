#include "test.h"
#include <fenv.h>

enum
{
    METHOD_COUNT = 4
};

/* Each method on {2^53, 1, -2^53} and on an empty input. */
static void check_methods_on_absorbed_term(void)
{
    double values[] = {0x1p53, 1, -0x1p53};
    double sentinel = 37;
    const double expected[METHOD_COUNT] = {0, 1, 1, 1};
    for (unsigned method = 0; method < METHOD_COUNT; method++)
    {
        CHECK(nd_sum(values, 3, method, &sentinel) == ND_OK);
        CHECK(sentinel == expected[method]);
        CHECK(nd_sum(NULL, 0, method, &sentinel) == ND_OK && sentinel == 0);
    }
}

static void check_compensated_methods(void)
{
    double out = 37;
    double cancelling[] = {1e16, 1, -1e16};
    CHECK(nd_sum(cancelling, 3, ND_KAHAN, &out) == ND_OK && out == 0);
    CHECK(nd_sum(cancelling, 3, ND_NEUMAIER, &out) == ND_OK && out == 1);
    double zero_sum[] = {1, -1};
    CHECK(nd_sum(zero_sum, 2, ND_NEUMAIER, &out) == ND_OK && out == 0);
}

/* A rejected call must leave *out untouched ("strong" output behaviour). */
static void check_rejections_keep_output(void)
{
    double out = 37;
    double with_nan[] = {1, NAN};
    CHECK(nd_sum(with_nan, 2, ND_NEUMAIER, &out) == ND_BAD && out == 37);
    double three[] = {0x1p53, 1, -0x1p53};
    CHECK(nd_sum(three, ND_SUM_MAX + 1, ND_NAIVE, &out) == ND_BAD && out == 37);
}

static void check_largest_exact_sum(void)
{
    double ones[ND_SUM_MAX];
    for (size_t i = 0; i < ND_SUM_MAX; i++)
    {
        ones[i] = 1;
    }
    double out = 0;
    for (unsigned method = 0; method < METHOD_COUNT; method++)
    {
        CHECK(nd_sum(ones, ND_SUM_MAX, method, &out) == ND_OK &&
              out == ND_SUM_MAX);
    }
}

int main(void)
{
    CHECK(fegetround() == FE_TONEAREST);
    check_methods_on_absorbed_term();
    check_compensated_methods();
    check_rejections_keep_output();
    check_largest_exact_sum();
    puts("PASS sums and nonfinite strong output");
}
