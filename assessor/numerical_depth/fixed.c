#include "numeric.h"
#include <math.h>
#ifndef ND_MUTANT
#define ND_MUTANT 0
#endif
static int admit(int16_t x, unsigned q)
{
    int m = 1 << q;
    return x >= -m && x < m;
}

static int16_t rounded(int64_t n, unsigned q)
{
    int64_t d = INT64_C(1) << q, m = n < 0 ? -n : n, r = (m + d / 2) / d;
    if (n < 0 && ND_MUTANT != 2)
    {
        r = -r;
    }
    int64_t hi = d - 1;
    if (r > hi)
    {
        r = hi;
    }
    if (r < -d)
    {
        r = -d;
    }
    return (int16_t)r;
}

int nd_qmul(int16_t a, int16_t b, unsigned q, int16_t *out)
{
    if (!out || (q != 7 && q != 11) || !admit(a, q) || !admit(b, q))
    {
        return ND_BAD;
    }
    *out = rounded((int64_t)a * b, q);
    return ND_OK;
}

int nd_qmac(const int16_t *a, const int16_t *b, size_t n, unsigned q,
            int16_t *out)
{
    if (!out || n > 32 || (!a && n) || (!b && n) || (q != 7 && q != 11))
    {
        return ND_BAD;
    }
    int64_t v = 0;
    for (size_t i = 0; i < n; i++)
    {
        if (!admit(a[i], q) || !admit(b[i], q))
        {
            return ND_BAD;
        }
        v += (int64_t)a[i] * b[i];
    }
    *out = rounded(v, q);
    return ND_OK;
}

int nd_quantize(double x, unsigned q, int16_t *out)
{
    if (!out || (q != 7 && q != 11) || !isfinite(x))
    {
        return ND_BAD;
    }
    int d = 1 << q;
    if (x >= 1)
    {
        *out = (int16_t)(d - 1);
        return ND_OK;
    }
    if (x <= -1)
    {
        *out = (int16_t)-d;
        return ND_OK;
    }
    double m = fabs(x) * d;
    int n = (int)floor(m + 0.5);
    if (x < 0)
    {
        n = -n;
    }
    if (n >= d)
    {
        n = d - 1;
    }
    *out = (int16_t)n;
    return ND_OK;
}
