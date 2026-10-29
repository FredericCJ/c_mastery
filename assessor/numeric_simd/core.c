#include "core.h"
#ifndef B06_MUTATION
#define B06_MUTATION 0
#endif

int ns_q15(int16_t a, int16_t b)
{
    int32_t p = (int32_t)a * b;
    int32_t m = p < 0 ? -p : p;
    int32_t r = (m + 16384) / 32768;
    if (p < 0)
    {
        r = -r;
    }
    if (B06_MUTATION == 1)
    {
        r = p / 32768;
    }
    if (r > 32767)
    {
        r = 32767;
    }
    if (r < -32768)
    {
        r = -32768;
    }
    return (int)r;
}

int ns_round_even(int n, unsigned d, int *o)
{
    unsigned m, q, r;
    if (n < -1000000 || n > 1000000 || d == 0 || d > 1000000)
    {
        return 0;
    }
    m = (unsigned)(n < 0 ? -n : n);
    q = m / d;
    r = m % d;
    if (r > d - r || (r == d - r && ((q & 1u) || B06_MUTATION == 2)))
    {
        ++q;
    }
    *o = n < 0 ? -(int)q : (int)q;
    return 1;
}

int ns_dot(const int16_t *a, const int16_t *b, size_t n, int64_t *o)
{
    size_t i;
    int64_t s = 0;
    if (n > 65535)
    {
        return 0;
    }
    for (i = 0; i < n; ++i)
    {
        s += (int64_t)a[i] * b[i];
    }
    if (B06_MUTATION == 3)
    {
        s = (int32_t)s;
    }
    *o = s;
    return 1;
}

int ns_sum(const double *a, size_t n, double *o)
{
    double s = 0, c = 0;
    size_t i;
    if (n > 65535)
    {
        return 0;
    }
    for (i = 0; i < n; ++i)
    {
        double x = a[i], t;
        if (!isfinite(x))
        {
            return 0;
        }
        t = s + x;
        if (!isfinite(t))
        {
            return 0;
        }
        if (B06_MUTATION != 4)
        {
            c += fabs(s) >= fabs(x) ? (s - t) + x : (x - t) + s;
        }
        if (!isfinite(c))
        {
            return 0;
        }
        s = t;
    }
    s += c;
    if (!isfinite(s))
    {
        return 0;
    }
    *o = s;
    return 1;
}

int ns_norm(const double *a, size_t n, double *o)
{
    double scale = 0, ss = 1, r, z;
    size_t i;
    if (n > 65535)
    {
        return 0;
    }
    for (i = 0; i < n; ++i)
    {
        double x = fabs(a[i]);
        if (!isfinite(x))
        {
            return 0;
        }
        if (x != 0)
        {
            if (scale < x)
            {
                r = scale / x;
                ss = 1 + ss * r * r;
                scale = x;
            }
            else
            {
                r = x / scale;
                ss += r * r;
            }
        }
    }
    z = scale == 0 ? 0 : scale * sqrt(ss);
    if (B06_MUTATION == 5 && n == 2)
    {
        z = sqrt(a[0] * a[0] + a[1] * a[1]);
    }
    if (!isfinite(z))
    {
        return 0;
    }
    *o = z;
    return 1;
}

int ns_matmul(const int32_t *a, const int32_t *b, unsigned r, unsigned k,
              unsigned c, int64_t *o)
{
    unsigned i, j, t;
    if (!r || !k || !c || r > 8 || k > 8 || c > 8)
    {
        return 0;
    }
    for (i = 0; i < r * k; ++i)
    {
        if (a[i] < -1024 || a[i] > 1024)
        {
            return 0;
        }
    }
    for (i = 0; i < k * c; ++i)
    {
        if (b[i] < -1024 || b[i] > 1024)
        {
            return 0;
        }
    }
    for (i = 0; i < r; ++i)
    {
        for (j = 0; j < c; ++j)
        {
            int64_t s = 0;
            for (t = 0; t < k; ++t)
            {
                s += (int64_t)a[i * k + t] *
                     b[t * c + (B06_MUTATION == 6 ? 0 : j)];
            }
            o[i * c + j] = s;
        }
    }
    return 1;
}

int ns_add(const int16_t *a, const int16_t *b, size_t n, int16_t *o)
{
    size_t i, end = n;
    if (n > 65535)
    {
        return 0;
    }
    if (B06_MUTATION == 7)
    {
        end = n - n % 4;
    }
    for (i = 0; i < end; ++i)
    {
        int32_t s = (int32_t)a[i] + b[i];
        if (s > 32767)
        {
            s = 32767;
        }
        if (s < -32768)
        {
            s = -32768;
        }
        o[i] = (int16_t)s;
    }
    return 1;
}

int ns_close(double a, double e, double at, double rt)
{
    double s, d, t;
    if (!isfinite(a) || !isfinite(e) || !isfinite(at) || !isfinite(rt) ||
        at < 0 || rt < 0 || rt > 1)
    {
        return 0;
    }
    s = fmax(fmax(fabs(a), fabs(e)), at);
    if (s == 0)
    {
        return 1;
    }
    d = fabs(a / s - e / s);
    t = at / s + rt * (fabs(e) / s);
    if (B06_MUTATION == 8)
    {
        t = rt * (fabs(e) / s);
    }
    return d <= t;
}
