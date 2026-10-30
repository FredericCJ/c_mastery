/*
 * Reference piecewise-linear interpolation.
 *
 * The whole knot array is validated before the interval search, so a bad
 * knot beyond the query still rejects the call. The segment is found by
 * binary search in O(log n); the value is (1 - t) * y_lo + t * y_hi with t
 * formed from the two adjacent knots, in exactly that evaluation order.
 *
 * Mutant ND_MUTANT == 4 stops enforcing strictly increasing knots.
 */
#include "numeric.h"
#include <math.h>
#ifndef ND_MUTANT
#define ND_MUTANT 0
#endif

/* 1 when every knot and ordinate is admitted and knots strictly increase. */
static int knots_admitted(const double *x, const double *y, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        int finite = isfinite(x[i]) && isfinite(y[i]);
        int bounded =
            fabs(x[i]) <= ND_VALUE_LIMIT && fabs(y[i]) <= ND_VALUE_LIMIT;
        int increasing = i == 0 || x[i] > x[i - 1] || ND_MUTANT == 4;
        if (!finite || !bounded || !increasing)
        {
            return 0;
        }
    }
    return 1;
}

/*
 * Index lo of the segment [x[lo], x[lo + 1]] containing q, for
 * x[0] <= q < x[n - 1]. Invariant: x[lo] <= q < x[hi].
 */
static size_t find_segment(const double *x, size_t n, double q)
{
    size_t lo = 0;
    size_t hi = n - 1;
    while (hi - lo > 1)
    {
        size_t mid = lo + (hi - lo) / 2;
        if (q < x[mid])
        {
            hi = mid;
        }
        else
        {
            lo = mid;
        }
    }
    return lo;
}

int nd_interp(const double *x, const double *y, size_t n, double q, int clamp,
              double *out)
{
    if (!out || !x || !y || !n || n > ND_INTERP_MAX || !isfinite(q) ||
        fabs(q) > ND_VALUE_LIMIT || (clamp != 0 && clamp != 1) ||
        !knots_admitted(x, y, n))
    {
        return ND_BAD;
    }
    if (q < x[0] || q > x[n - 1])
    {
        if (!clamp)
        {
            return ND_RANGE;
        }
        *out = q < x[0] ? y[0] : y[n - 1];
        return ND_OK;
    }
    /* The upper endpoint returns its stored ordinate, never a computed
     * blend. */
    if (n == 1 || q == x[n - 1])
    {
        *out = y[n - 1];
        return ND_OK;
    }
    size_t lo = find_segment(x, n, q);
    size_t hi = lo + 1;
    double t = (q - x[lo]) / (x[hi] - x[lo]);
    *out = (1 - t) * y[lo] + t * y[hi];
    return ND_OK;
}
