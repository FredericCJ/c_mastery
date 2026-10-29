#include "numeric.h"
#include <float.h>
#include <math.h>
_Static_assert(FLT_RADIX == 2 && DBL_MANT_DIG == 53 && DBL_MAX_EXP == 1024,
               "binary64 required");
#ifndef ND_MUTANT
#define ND_MUTANT 0
#endif
static double pair(const double *x, size_t n)
{
    if (n == 0)
        return 0;
    if (n == 1)
        return x[0];
    size_t k = n / 2;
    return pair(x, k) + pair(x + k, n - k);
}

int nd_sum(const double *x, size_t n, unsigned method, double *out)
{
    if (!out || (!x && n) || n > 4096 || method > ND_NEUMAIER)
        return ND_BAD;
    for (size_t i = 0; i < n; i++)
        if (!isfinite(x[i]) || fabs(x[i]) > 1e100)
            return ND_BAD;
    double s = 0, c = 0;
    if (method == ND_PAIRWISE)
        s = pair(x, n);
    else
        for (size_t i = 0; i < n; i++)
        {
            if (method == ND_NAIVE)
                s += x[i];
            else if (method == ND_KAHAN)
            {
                double old = s, y = x[i] + c;
                s = old + y;
                c = (old - s) + y;
            }
            else
            {
                double t = s + x[i];
                if (ND_MUTANT != 1)
                    c +=
                        fabs(s) >= fabs(x[i]) ? (s - t) + x[i] : (x[i] - t) + s;
                s = t;
            }
        }
    if (method == ND_NEUMAIER)
        s += c;
    *out = s;
    return ND_OK;
}
