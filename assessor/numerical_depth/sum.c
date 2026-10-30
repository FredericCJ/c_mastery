/*
 * Reference summation: naive, pairwise, Kahan and Neumaier.
 *
 * Every method adds in a fixed order and relies on binary64 round-to-nearest
 * arithmetic without contraction or reassociation: the unit tests build with
 * -ffp-contract=off -fno-fast-math. (The campaign's relaxed environment build
 * deliberately compiles this file with fast-math to observe the difference.)
 * Reordering any expression below changes the rounded result.
 *
 * Mutant ND_MUTANT == 1 drops the Neumaier correction term.
 */
#include "numeric.h"
#include <float.h>
#include <math.h>
_Static_assert(FLT_RADIX == 2 && DBL_MANT_DIG == 53 && DBL_MAX_EXP == 1024,
               "binary64 required");
#ifndef ND_MUTANT
#define ND_MUTANT 0
#endif

/* 1 when every value is finite and within ND_VALUE_LIMIT. */
static int values_admitted(const double *x, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        if (!isfinite(x[i]) || fabs(x[i]) > ND_VALUE_LIMIT)
        {
            return 0;
        }
    }
    return 1;
}

/*
 * Fixed tree: split at floor(n / 2) and combine the halves' sums. The tree
 * shape (including odd lengths) is part of the bitwise contract. Recursion
 * depth is at most log2(ND_SUM_MAX) + 1 = 13.
 */
static double sum_pairwise(const double *x, size_t n)
{
    if (n == 0)
    {
        return 0;
    }
    if (n == 1)
    {
        return x[0];
    }
    size_t half = n / 2;
    return sum_pairwise(x, half) + sum_pairwise(x + half, n - half);
}

/* Left to right, starting from +0. */
static double sum_naive(const double *x, size_t n)
{
    double sum = 0;
    for (size_t i = 0; i < n; i++)
    {
        sum += x[i];
    }
    return sum;
}

/*
 * Kahan's form: carry the rounding error of each addition into the next
 * input. The final correction is not added back (that is Neumaier's step).
 */
static double sum_kahan(const double *x, size_t n)
{
    double sum = 0;
    double correction = 0;
    for (size_t i = 0; i < n; i++)
    {
        double old_sum = sum;
        double adjusted = x[i] + correction;
        sum = old_sum + adjusted;
        correction = (old_sum - sum) + adjusted;
    }
    return sum;
}

/*
 * Neumaier's form: the error of sum + x[i] is recovered from whichever operand
 * has the larger magnitude, accumulated separately and added once at the end.
 */
static double sum_neumaier(const double *x, size_t n)
{
    double sum = 0;
    double correction = 0;
    for (size_t i = 0; i < n; i++)
    {
        double total = sum + x[i];
        if (ND_MUTANT != 1)
        {
            correction += fabs(sum) >= fabs(x[i]) ? (sum - total) + x[i]
                                                  : (x[i] - total) + sum;
        }
        sum = total;
    }
    sum += correction;
    return sum;
}

int nd_sum(const double *x, size_t n, unsigned method, double *out)
{
    if (!out || (!x && n) || n > ND_SUM_MAX || method > ND_NEUMAIER ||
        !values_admitted(x, n))
    {
        return ND_BAD;
    }
    double sum;
    if (method == ND_PAIRWISE)
    {
        sum = sum_pairwise(x, n);
    }
    else if (method == ND_KAHAN)
    {
        sum = sum_kahan(x, n);
    }
    else if (method == ND_NEUMAIER)
    {
        sum = sum_neumaier(x, n);
    }
    else
    {
        sum = sum_naive(x, n);
    }
    *out = sum;
    return ND_OK;
}
