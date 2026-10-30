/*
 * Reference Euler stepping and first-order filter. Both stage their results in
 * a local array and copy to the caller only after every step succeeded, so a
 * rejected call leaves the caller's output (and, for nd_iir, state) untouched.
 * Because nd_iir stages, its output may overlap its input.
 *
 * Mutant ND_MUTANT == 6 publishes the initial value as the final Euler step;
 * mutant 8 ignores the carried filter state.
 */
#include "numeric.h"
#include <math.h>
#ifndef ND_MUTANT
#define ND_MUTANT 0
#endif

/* 1 when v is finite and within the admitted magnitude. */
static int value_admitted(double v)
{
    return isfinite(v) && fabs(v) <= ND_VALUE_LIMIT;
}

int nd_euler(double y, double h, double rate, size_t n, double *out, size_t cap)
{
    if (!out || n > ND_STREAM_MAX || cap < n + 1 || !value_admitted(y) ||
        !isfinite(h) || !isfinite(rate) || h <= 0 || rate <= 0 ||
        h > ND_VALUE_LIMIT || rate > ND_VALUE_LIMIT)
    {
        return ND_BAD;
    }
    double staged[ND_STREAM_MAX + 1];
    staged[0] = y;
    for (size_t i = 0; i < n; i++)
    {
        /* Forward Euler for y' = -rate * y; keep this evaluation order. */
        y = y + h * (-rate * y);
        if (!isfinite(y))
        {
            return ND_RANGE;
        }
        staged[i + 1] = y;
    }
    if (ND_MUTANT == 6)
    {
        staged[n] = staged[0];
    }
    for (size_t i = 0; i <= n; i++)
    {
        out[i] = staged[i];
    }
    return ND_OK;
}

int nd_iir(double *state, const double *in, size_t n, double feed,
           double feedback, double *out, size_t cap)
{
    if (!state || (!in && n) || (!out && n) || n > ND_STREAM_MAX || cap < n ||
        !value_admitted(*state) || !isfinite(feed) || fabs(feed) > 1 ||
        !isfinite(feedback) || fabs(feedback) >= 1)
    {
        return ND_BAD;
    }
    double staged[ND_STREAM_MAX];
    double s = *state;
    if (ND_MUTANT == 8)
    {
        s = 0;
    }
    for (size_t i = 0; i < n; i++)
    {
        if (!value_admitted(in[i]))
        {
            return ND_BAD;
        }
        /* One first-order step: s' = feed * x + feedback * s. */
        s = feed * in[i] + feedback * s;
        staged[i] = s;
    }
    for (size_t i = 0; i < n; i++)
    {
        out[i] = staged[i];
    }
    *state = s;
    return ND_OK;
}
