#include <stddef.h>
#include <math.h>

double ff_expression(double a, double b, double c)
{
    return a * b + c;
}

double ff_separate(double a, double b, double c)
{
    volatile double product = a * b;
    return product + c;
}

double ff_zero(double a)
{
    return a + 0.0;
}

double ff_missing(const double *a, size_t n)
{
    double s = 0;
    for (size_t i = 0; i < n; i++)
    {
        if (!isnan(a[i]))
        {
            s += a[i];
        }
    }
    return s;
}

double ff_kernel(const double *a, size_t n)
{
    double s = 0;
    for (size_t i = 0; i < n; i++)
    {
        s += a[i];
    }
    return s;
}
