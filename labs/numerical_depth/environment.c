/*
 * Floating-point environment observations: rounding modes, fused versus
 * separate multiply-add, and how splitting a sum across workers changes the
 * rounded result. Prints one JSON object of hexadecimal ("%a") values.
 *
 * The campaign builds this file twice, with strict flags and with relaxed
 * (fast-math) flags, and records both outputs separately; the two builds have
 * different arithmetic semantics. Exit status 2 when a rounding mode cannot be
 * set, 3 when a worker thread cannot be started or joined, 4 when the
 * compensated sum fails.
 */
#include "numeric.h"
#include <fenv.h>
#include <float.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define ROUNDING_MODES 4u
#define VALUES 3u
#define MAX_WORKERS 3u

/* A slice of the input and the naive sum of that slice. */
struct nd_work
{
    const double *x;
    size_t n;
    double result;
};

static void *worker(void *arg)
{
    struct nd_work *work = arg;
    if (nd_sum(work->x, work->n, ND_NAIVE, &work->result))
    {
        abort();
    }
    return NULL;
}

/*
 * Starts one thread per slice, then joins all of them, so every result is
 * published before the caller reads it. Returns 0, or 3 on a thread error.
 * Only count <= MAX_WORKERS is supported.
 */
static int run_workers(struct nd_work *slices, size_t count)
{
    pthread_t threads[MAX_WORKERS];
    for (size_t i = 0; i < count; i++)
    {
        if (pthread_create(threads + i, NULL, worker, slices + i))
        {
            return 3;
        }
    }
    for (size_t i = 0; i < count; i++)
    {
        if (pthread_join(threads[i], NULL))
        {
            return 3;
        }
    }
    return 0;
}

/* 1 + 2^-53 under each of the four rounding modes; restores
 * round-to-nearest. */
static int sample_rounding_modes(double rounded[ROUNDING_MODES])
{
    volatile double one = 1;
    volatile double small = 0x1p-53;
    const int modes[ROUNDING_MODES] = {FE_TONEAREST, FE_UPWARD, FE_DOWNWARD,
                                       FE_TOWARDZERO};
    for (size_t i = 0; i < ROUNDING_MODES; i++)
    {
        if (fesetround(modes[i]))
        {
            return 2;
        }
        rounded[i] = one + small;
    }
    if (fesetround(FE_TONEAREST))
    {
        return 2;
    }
    return 0;
}

/*
 * Sums the VALUES inputs as two joined slices, split after `split` values
 * (1 or 2), and adds the two partial sums.
 */
static int two_worker_sum(const double *values, size_t split, double *total)
{
    struct nd_work slices[2] = {{values, split, 0},
                                {values + split, VALUES - split, 0}};
    int status = run_workers(slices, 2);
    if (status)
    {
        return status;
    }
    *total = slices[0].result + slices[1].result;
    return 0;
}

static int three_worker_sum(const double *values, double *total)
{
    struct nd_work slices[VALUES] = {
        {values, 1, 0}, {values + 1, 1, 0}, {values + 2, 1, 0}};
    int status = run_workers(slices, VALUES);
    if (status)
    {
        return status;
    }
    *total = (slices[0].result + slices[1].result) + slices[2].result;
    return 0;
}

int main(void)
{
    double rounded[ROUNDING_MODES];
    int status = sample_rounding_modes(rounded);
    if (status)
    {
        return status;
    }

    /* (1 + 2^-27)(1 - 2^-27) - 1: product rounded first, or fused. */
    volatile double a = 1 + 0x1p-27;
    volatile double b = 1 - 0x1p-27;
    volatile double c = -1;
    double separate = a * b + c;
    double fused = fma(a, b, c);

    double values[VALUES] = {0x1p53, 1, -0x1p53};
    double reductions[2];
    for (size_t split = 1; split <= 2; split++)
    {
        status = two_worker_sum(values, split, &reductions[split - 1]);
        if (status)
        {
            return status;
        }
    }
    double compensated;
    if (nd_sum(values, VALUES, ND_NEUMAIER, &compensated))
    {
        return 4;
    }
    double three_workers;
    status = three_worker_sum(values, &three_workers);
    if (status)
    {
        return status;
    }
#ifdef __FAST_MATH__
    const int fast = 1;
#else
    const int fast = 0;
#endif
    printf(
        "{\"fast_math\":%d,\"flt_eval_method\":%d,\"rounding\":[\"%a\",\"%a\","
        "\"%a\",\"%a\"],\"separate\":\"%a\",\"fma\":\"%a\",\"two_worker_"
        "splits\":[\"%a\",\"%a\"],\"compensated\":\"%a\",\"three_workers\":\"%"
        "a\"}\n",
        fast, FLT_EVAL_METHOD, rounded[0], rounded[1], rounded[2], rounded[3],
        separate, fused, reductions[0], reductions[1], compensated,
        three_workers);
    return 0;
}
