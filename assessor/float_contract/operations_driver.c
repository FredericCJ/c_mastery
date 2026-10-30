/*
 * Strict caller for the kernels in operations.c, which are linked from objects
 * built with varying compiler options. The caller itself is always built with
 * strict floating-point options, so differences in the printed results come
 * from the kernels' compilation, not from this file.
 *
 * It prints one OPERATIONS line of results for the experiment script to parse,
 * then five raw timing trials of the sum kernel on an exactly representable
 * finite workload. Timings are local observations, not benchmarks.
 */
#define _POSIX_C_SOURCE 200809L
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stddef.h>
#include <time.h>
#include <fenv.h>
#ifdef __clang__
#pragma STDC FENV_ACCESS ON
#endif

double ff_expression(double a, double b, double c);
double ff_separate(double a, double b, double c);
double ff_zero(double a);
double ff_missing(const double *a, size_t n);
double ff_kernel(const double *a, size_t n);

#define MISSING_COUNT 3u
#define FINITE_COUNT 64u
#define TIMING_TRIALS 5
#define CALLS_PER_TRIAL 32768
#define NANOSECONDS_PER_SECOND 1000000000LL

static volatile double sink; /* keeps the timed calls from being removed */

/* Copy the finite elements of in[0..n) to out and return how many. out must
 * have room for n elements. */
static size_t keep_finite(const double *in, size_t n, double *out)
{
    size_t used = 0;
    for (size_t i = 0; i < n; i++)
    {
        if (isfinite(in[i]))
        {
            out[used++] = in[i];
        }
    }
    return used;
}

/* Exact dyadic workload: (i - 32) * 2^-10 for i in 0..63. Every value and every
 * partial sum is exactly representable, so any summation order gives the same
 * answer and the result can be compared with ==. Returns the exact sum. */
static double fill_exact_workload(double finite[FINITE_COUNT])
{
    double expected = 0;
    for (size_t i = 0; i < FINITE_COUNT; i++)
    {
        finite[i] = ((int)i - 32) * 0x1p-10;
        expected += finite[i];
    }
    return expected;
}

/* One timing trial of CALLS_PER_TRIAL calls; prints a TIMING line. Returns 0,
 * or 1 if the clock fails, the result is wrong or time runs backwards. */
static int time_trial(int trial, const double finite[FINITE_COUNT],
                      double expected)
{
    struct timespec start;
    struct timespec end;
    if (clock_gettime(CLOCK_MONOTONIC, &start) != 0)
    {
        return 1;
    }
    for (int i = 0; i < CALLS_PER_TRIAL; i++)
    {
        sink = ff_kernel(finite, FINITE_COUNT);
    }
    if (clock_gettime(CLOCK_MONOTONIC, &end) != 0)
    {
        return 1;
    }
    long long elapsed_ns =
        ((long long)(end.tv_sec - start.tv_sec) * NANOSECONDS_PER_SECOND) +
        end.tv_nsec - start.tv_nsec;
    if ((sink != expected) || (elapsed_ns < 0))
    {
        return 1;
    }
    printf("TIMING trial=%d nanoseconds=%lld calls=%d count=%u exact=%a\n",
           trial, elapsed_ns, CALLS_PER_TRIAL, FINITE_COUNT, expected);
    return 0;
}

int main(void)
{
    if (fesetround(FE_TONEAREST) != 0)
    {
        return 1;
    }

    /* volatile: the operands must be run-time values so each kernel's own
     * compilation, not constant folding here, decides the result. */
    volatile double a = 1 + 0x1p-27;
    volatile double b = 1 - 0x1p-27;
    volatile double c = -1;
    double expression = ff_expression(a, b, c);
    double separate = ff_separate(a, b, c);

    const double missing[MISSING_COUNT] = {1, NAN, 2};
    double filtered[MISSING_COUNT];
    size_t used = keep_finite(missing, MISSING_COUNT, filtered);
    double repaired = ff_kernel(filtered, used);
    double raw = ff_missing(missing, MISSING_COUNT);
    double zero = ff_zero(-0.0);
    const double cancellation_input[] = {1e16, 1, -1e16};

    printf(
        "OPERATIONS expression=%a separate=%a missing_nan=%d raw_missing=%a repaired=%a zero_sign=%d cancellation=%a eval=%d\n",
        expression, separate, !!isnan(raw), raw, repaired, !!signbit(zero),
        ff_kernel(cancellation_input, 3), FLT_EVAL_METHOD);
    if ((separate != 0) || (repaired != 3))
    {
        return 1;
    }

    double finite[FINITE_COUNT];
    double expected = fill_exact_workload(finite);
    if (ff_kernel(finite, FINITE_COUNT) != expected)
    {
        return 1;
    }
    for (int trial = 0; trial < TIMING_TRIALS; trial++)
    {
        if (time_trial(trial, finite, expected) != 0)
        {
            return 1;
        }
    }
    puts(
        "FF_OPERATIONS_PASS strict_caller finite_admission nan_filter_repair raw_timings");
    return 0;
}
