#include "numeric_transfer.h"

int nt_dot(const int16_t *a, const int16_t *b, size_t n, int64_t *out,
           unsigned mode)
{
    (void)a;
    (void)b;
    (void)n;
    (void)out;
    (void)mode;
    return 0;
}

int nt_parallel(const double *a, size_t n, double *out, unsigned s)
{
    (void)a;
    (void)n;
    (void)out;
    (void)s;
    return 0;
}

int nt_ulp(double a, double b, uint64_t *out)
{
    (void)a;
    (void)b;
    (void)out;
    return 0;
}

int nt_close(double a, double b, double x, double r)
{
    (void)a;
    (void)b;
    (void)x;
    (void)r;
    return 0;
}
