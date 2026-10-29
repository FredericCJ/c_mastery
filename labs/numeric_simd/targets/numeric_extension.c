#include "numeric_extension.h"

int nd_q312(int16_t a, int16_t b, unsigned m, int16_t *o)
{
    (void)a;
    (void)b;
    (void)m;
    (void)o;
    return 0;
}

int nd_dot(const int16_t *a, const int16_t *b, size_t n, int64_t *o, unsigned l)
{
    (void)a;
    (void)b;
    (void)n;
    (void)o;
    (void)l;
    return 0;
}

int nd_accumulate(int64_t *s, int64_t p)
{
    (void)s;
    (void)p;
    return 0;
}

int nd_pairwise(const double *a, size_t n, double *o)
{
    (void)a;
    (void)n;
    (void)o;
    return 0;
}

int nd_matrix(const double *a, const double *b, size_t m, size_t n, size_t k,
              double *o, size_t c, unsigned v)
{
    (void)a;
    (void)b;
    (void)m;
    (void)n;
    (void)k;
    (void)o;
    (void)c;
    (void)v;
    return 0;
}
