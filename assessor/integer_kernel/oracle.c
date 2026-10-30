/*
 * Independent oracle for integer.h on small synthetic ranges.
 *
 * Every range here fits native int arithmetic, so the oracle can form exact
 * sums and products directly, independently of the checked guards in the
 * reference. Division is judged by a unique mathematical specification (the
 * identity a == q * b + r, a sign rule, and |r| < |b|) found by exhaustive
 * search over candidate quotients, not by a second copy of the
 * quotient-adjustment code.
 */
#include "integer.h"
#include <stdbool.h>
#include <stdio.h>

/* a, b and op must be in scope at the point of use; they identify the case. */
#define REQUIRE(x)                                                      \
    do                                                                  \
    {                                                                   \
        if (!(x))                                                       \
        {                                                               \
            fprintf(stderr, "FI_ORACLE_FAIL line=%d a=%d b=%d op=%u\n", \
                    __LINE__, a, b, op);                                \
            return 1;                                                   \
        }                                                               \
    } while (0)

/* Sentinels: outputs the function under test must leave alone on failure. */
#define UNTOUCHED_VALUE 917
#define UNTOUCHED_REMAINDER 919
#define UNTOUCHED_CLIPPED 99u

#define OPERATION_COUNT 3u

struct oracle_counts
{
    unsigned long arithmetic;
    unsigned long saturation;
    unsigned long division;
};

static int magnitude(int n)
{
    return (n < 0) ? -n : n;
}

/* Exact a (op) b. The largest magnitude is 128 * 128 = 16384, which every
 * conforming int (at least 16 bits) represents. */
static int exact_result(unsigned op, int a, int b)
{
    if (op == (unsigned)FI_ADD)
    {
        return a + b;
    }
    if (op == (unsigned)FI_SUBTRACT)
    {
        return a - b;
    }
    return a * b;
}

static int clamp_to_range(struct fi_range range, int exact)
{
    if (exact < range.low)
    {
        return range.low;
    }
    if (exact > range.high)
    {
        return range.high;
    }
    return exact;
}

static int check_arithmetic(struct fi_range range, unsigned op, int a, int b,
                            struct oracle_counts *counts)
{
    int exact = exact_result(op, a, b);
    bool admitted = (exact >= range.low) && (exact <= range.high);
    int out = UNTOUCHED_VALUE;

    REQUIRE(fi_binary(range, (enum fi_operation)op, a, b, &out) ==
            (admitted ? FI_OK : FI_RANGE));
    REQUIRE(out == (admitted ? exact : UNTOUCHED_VALUE));
    ++counts->arithmetic;

    unsigned clipped = UNTOUCHED_CLIPPED;
    out = UNTOUCHED_VALUE;
    REQUIRE(fi_saturate(range, (enum fi_operation)op, a, b, &out, &clipped) ==
            FI_OK);
    REQUIRE(out == clamp_to_range(range, exact));
    REQUIRE(clipped == (admitted ? 0u : 1u));
    ++counts->saturation;
    return 0;
}

/* Does the pair (candidate, residual) satisfy the sign rule of the policy?
 * Truncation: the remainder follows the dividend. Floor: it follows the
 * divisor. Euclidean: it is never negative. A zero remainder satisfies the
 * first two. */
static bool remainder_sign_matches(enum fi_division policy, int a, int b,
                                   int residual)
{
    if (policy == FI_TRUNCATE)
    {
        return (residual == 0) || ((residual < 0) == (a < 0));
    }
    if (policy == FI_FLOOR)
    {
        return (residual == 0) || ((residual < 0) == (b < 0));
    }
    return residual >= 0;
}

/*
 * Search the range for the quotient satisfying the policy. On the asymmetric
 * small range some otherwise valid quotients or remainders lie outside the
 * range, so existence is itself part of the answer. *found is set to whether
 * exactly one candidate exists; two candidates are an oracle failure.
 */
static int search_division(struct fi_range range, unsigned op, int a, int b,
                           bool *found, int *wanted_quotient,
                           int *wanted_remainder)
{
    *found = false;
    *wanted_quotient = UNTOUCHED_VALUE;
    *wanted_remainder = UNTOUCHED_REMAINDER;
    if (b == 0)
    {
        return 0;
    }

    for (int candidate = range.low; candidate <= range.high; ++candidate)
    {
        int residual = a - candidate * b;
        if ((residual < range.low) || (residual > range.high) ||
            (magnitude(residual) >= magnitude(b)))
        {
            continue;
        }
        if (remainder_sign_matches((enum fi_division)op, a, b, residual))
        {
            REQUIRE(!*found);
            *found = true;
            *wanted_quotient = candidate;
            *wanted_remainder = residual;
        }
    }
    return 0;
}

static int check_division(struct fi_range range, unsigned op, int a, int b,
                          struct oracle_counts *counts)
{
    bool found = false;
    int wanted_quotient = 0;
    int wanted_remainder = 0;
    if (search_division(range, op, a, b, &found, &wanted_quotient,
                        &wanted_remainder) != 0)
    {
        return 1;
    }

    int q = UNTOUCHED_VALUE;
    int r = UNTOUCHED_REMAINDER;
    /* The baseline rejects an unrepresentable truncating quotient before
     * either / or %, even where another application's rounding convention
     * could otherwise differ. */
    REQUIRE(fi_divide(range, (enum fi_division)op, a, b, &q, &r) ==
            (found ? FI_OK : FI_RANGE));
    REQUIRE(q == wanted_quotient && r == wanted_remainder);
    ++counts->division;
    return 0;
}

int main(void)
{
    const struct fi_range ranges[] = {{-128, 127}, {-127, 127}, {-12, 3}};
    struct oracle_counts counts = {0, 0, 0};

    for (unsigned k = 0; k < sizeof ranges / sizeof ranges[0]; ++k)
    {
        const struct fi_range range = ranges[k];
        for (int a = range.low; a <= range.high; ++a)
        {
            for (int b = range.low; b <= range.high; ++b)
            {
                for (unsigned op = 0; op < OPERATION_COUNT; ++op)
                {
                    if (check_arithmetic(range, op, a, b, &counts) != 0)
                    {
                        return 1;
                    }
                }
                for (unsigned op = 0; op < OPERATION_COUNT; ++op)
                {
                    if (check_division(range, op, a, b, &counts) != 0)
                    {
                        return 1;
                    }
                }
            }
        }
    }
    printf("FI_ORACLE_PASS arithmetic=%lu saturation=%lu division=%lu\n",
           counts.arithmetic, counts.saturation, counts.division);
    return 0;
}
