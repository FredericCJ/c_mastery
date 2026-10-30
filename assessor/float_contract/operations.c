/*
 * Small kernels compiled alone, with different compiler options, to show how
 * the options change the rounding points and exceptional-value behaviour:
 * strict (no contraction), contracted (fused multiply-add), x87 (wider
 * intermediates) and fast-math (NaN and signed-zero assumptions). Nothing
 * here is correct under every option set; the campaign records what each
 * option set actually does. They are called from operations_driver.c, which is
 * itself built with strict options.
 */
#include <stddef.h>
#include <math.h>

/* a * b + c as one expression: a compiler may fuse it, or keep the product at
 * wider precision, so the result can differ by the product's rounding error. */
double ff_expression(double a, double b, double c)
{
    return a * b + c;
}

/* Same value with the product forced to a double first (volatile store), so no
 * fusing or wider intermediate can occur. */
double ff_separate(double a, double b, double c)
{
    volatile double product = a * b;
    return product + c;
}

/* a + 0.0 turns -0.0 into +0.0 under strict rules; fast-math may delete the
 * addition and keep the sign. */
double ff_zero(double a)
{
    return a + 0.0;
}

/* Sum that skips NaN elements. Under fast-math the compiler may assume no NaN
 * exists and remove the test, so the NaN leaks into the result. */
double ff_missing(const double *a, size_t n)
{
    double sum = 0;
    for (size_t i = 0; i < n; i++)
    {
        if (!isnan(a[i]))
        {
            sum += a[i];
        }
    }
    return sum;
}

/* Plain left-to-right sum of n finite elements. */
double ff_kernel(const double *a, size_t n)
{
    double sum = 0;
    for (size_t i = 0; i < n; i++)
    {
        sum += a[i];
    }
    return sum;
}
