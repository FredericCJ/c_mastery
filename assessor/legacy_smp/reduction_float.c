#include "reduction_float.h"
#include <float.h>
#ifdef __FAST_MATH__
#error This experiment requires no fast-math reassociation
#endif
_Static_assert(FLT_RADIX == 2 && DBL_MANT_DIG == 53 && DBL_MAX_EXP == 1024,
               "Selected floating experiment requires binary64");

static double sf_abs(double v)
{
    return v < 0 ? -v : v;
}

static void sf_add(struct sr_float *p, double v, int compensated)
{
    double sum = p->sum + v;
    if (compensated)
    {
        p->correction += sf_abs(p->sum) >= sf_abs(v) ? (p->sum - sum) + v
                                                     : (v - sum) + p->sum;
    }
    p->sum = sum;
}

int sr_float_chunk(const double *values, size_t n, unsigned workers,
                   unsigned worker, int compensated, struct sr_float *out)
{
    if (!out || (n && !values) || (compensated != 0 && compensated != 1))
    {
        return SR_INVALID;
    }
    struct sr_range range;
    int result = sr_partition(n, workers, worker, &range);
    if (result != SR_OK)
    {
        return result;
    }
    struct sr_float value = {0};
    for (size_t i = range.begin; i < range.end; i++)
    {
        sf_add(&value, values[i], compensated);
    }
    *out = value;
    return SR_OK;
}

int sr_float_join(const struct sr_float *parts, unsigned workers,
                  int compensated, double *out)
{
    if (!parts || !out || !workers || workers > 4 ||
        (compensated != 0 && compensated != 1))
    {
        return SR_INVALID;
    }
    struct sr_float value = {0};
    for (unsigned i = 0; i < workers; i++)
    {
        sf_add(&value, parts[i].sum, compensated);
        if (compensated)
        {
            sf_add(&value, parts[i].correction, compensated);
        }
    }
    *out = value.sum + (compensated ? value.correction : 0);
    return SR_OK;
}
