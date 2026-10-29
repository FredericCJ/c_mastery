#include "numeric.h"
#include <math.h>
#ifndef ND_MUTANT
#define ND_MUTANT 0
#endif
int nd_interp(const double *x, const double *y, size_t n, double q, int clamp,
              double *out)
{
    if (!out || !x || !y || !n || n > 256 || !isfinite(q) || fabs(q) > 1e100 ||
        (clamp != 0 && clamp != 1))
        return ND_BAD;
    for (size_t i = 0; i < n; i++)
        if (!isfinite(x[i]) || !isfinite(y[i]) || fabs(x[i]) > 1e100 ||
            fabs(y[i]) > 1e100 || (i && x[i] <= x[i - 1] && ND_MUTANT != 4))
            return ND_BAD;
    if (q < x[0] || q > x[n - 1])
    {
        if (!clamp)
            return ND_RANGE;
        *out = q < x[0] ? y[0] : y[n - 1];
        return ND_OK;
    }
    if (n == 1 || q == x[n - 1])
    {
        *out = y[n - 1];
        return ND_OK;
    }
    size_t lo = 0, hi = n - 1;
    while (hi - lo > 1)
    {
        size_t mid = lo + (hi - lo) / 2;
        if (q < x[mid])
            hi = mid;
        else
            lo = mid;
    }
    double t = (q - x[lo]) / (x[hi] - x[lo]);
    *out = (1 - t) * y[lo] + t * y[hi];
    return ND_OK;
}
