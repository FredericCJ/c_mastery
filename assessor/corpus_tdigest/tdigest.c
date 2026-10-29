#include "tdigest.h"
#include <float.h>
#ifndef TD_MUTANT
#define TD_MUTANT 0
#endif
_Static_assert(DBL_MANT_DIG >= 53 && DBL_MAX_EXP >= 1024,
               "53-bit double required");

static int finite_value(double x)
{
    return x >= -1e9 && x <= 1e9;
}

static double ln_positive(double x)
{
    int exponent = 0;
    unsigned i;
    double z, term, total = 0;
    while (x >= 2)
    {
        x *= .5;
        exponent++;
    }
    while (x < 1)
    {
        x *= 2;
        exponent--;
    }
    z = (x - 1) / (x + 1);
    term = z;
    for (i = 0; i < 24; i++)
    {
        total += term / (double)(2 * i + 1);
        term *= z * z;
    }
    return 2 * total + (double)exponent * .69314718055994530942;
}

static double span(unsigned delta, uint32_t total, uint32_t left,
                   uint32_t weight)
{
    uint32_t right = left + weight;
    double factor =
        (double)delta / (4 * ln_positive((double)total / (double)delta) + 24);
    return factor *
           (ln_positive((double)right / (double)left) +
            ln_positive((double)(total - left) / (double)(total - right)));
}

static int admitted(unsigned delta, uint32_t n, uint32_t left, uint32_t weight,
                    double limit)
{
    if (weight == 1)
        return 1;
    if (!left || left + weight == n)
        return 0;
    return span(delta, n, left, weight) <= limit;
}

int td_valid(const struct td_td *s)
{
    unsigned i;
    uint32_t weight = 0;
    if (s->compression < 8 || s->compression > 32 || s->used > 128 ||
        s->count > 65536 || (!s->used) != (s->count == 0))
        return 0;
    for (i = 0; i < s->used; i++)
    {
        const struct td_centroid *c = &s->centroids[i];
        if (!c->weight || c->weight > s->count - weight ||
            !finite_value(c->mean) || (i && c->mean < s->centroids[i - 1].mean))
            return 0;
        if (!admitted(s->compression, s->count, weight, c->weight, 1.0 + 1e-10))
            return 0;
        weight += c->weight;
    }
    return weight == s->count;
}

int td_init(struct td_td *s, unsigned delta)
{
    struct td_td next = {0};
    if (delta < 8 || delta > 32)
        return TD_INVALID;
    next.compression = delta;
    *s = next;
    return TD_OK;
}

static int consolidate(struct td_td *out, struct td_centroid *input,
                       unsigned count, uint32_t total, unsigned delta)
{
    struct td_td next = {0};
    unsigned i, j;
    uint32_t left = 0;
    next.compression = delta;
    next.count = total;
    /* Stable insertion sort; equal means retain destination-before-source order. */
    for (i = 1; i < count; i++)
    {
        struct td_centroid item = input[i];
        j = i;
        while (j && input[j - 1].mean > item.mean)
        {
            input[j] = input[j - 1];
            j--;
        }
        input[j] = item;
    }
    for (i = 0; i < count; i++)
    {
        if (next.used)
        {
            struct td_centroid *last = &next.centroids[next.used - 1];
            uint32_t w = last->weight + input[i].weight;
            if (TD_MUTANT == 1 || admitted(delta, total, left, w, 1.0 - 1e-10))
            {
                double v = last->mean + (input[i].mean - last->mean) *
                                            (double)input[i].weight / (double)w;
                if (TD_MUTANT == 3)
                    v = (last->mean + input[i].mean) * .5;
                if (v < last->mean)
                    v = last->mean;
                if (v > input[i].mean)
                    v = input[i].mean;
                last->mean = v;
                last->weight = w;
                continue;
            }
            left += last->weight;
        }
        if (next.used == 128)
            return TD_CAPACITY;
        next.centroids[next.used++] = input[i];
    }
    if (!td_valid(&next))
        return TD_INVALID;
    *out = next;
    return TD_OK;
}

int td_add(struct td_td *s, const double *values, size_t n)
{
    struct td_centroid input[192];
    unsigned i;
    if (!td_valid(s) || n > 64 || (n && !values))
        return TD_INVALID;
    if (n > 65536u - s->count)
        return TD_CAPACITY;
    for (i = 0; i < n; i++)
        if (!finite_value(values[i]))
            return TD_INVALID;
    if (!n)
        return TD_OK;
    for (i = 0; i < s->used; i++)
        input[i] = s->centroids[i];
    for (i = 0; i < n; i++)
    {
        input[s->used + i].mean = values[i];
        input[s->used + i].weight = 1;
    }
    return consolidate(s, input, s->used + (unsigned)n, s->count + (uint32_t)n,
                       s->compression);
}

int td_merge(struct td_td *dst, const struct td_td *src)
{
    struct td_centroid input[256];
    unsigned i;
    if (!td_valid(dst) || !td_valid(src) || dst->compression > src->compression)
        return TD_INVALID;
    if (src->count > 65536u - dst->count)
        return TD_CAPACITY;
    if (!src->count)
        return TD_OK;
    for (i = 0; i < dst->used; i++)
        input[i] = dst->centroids[i];
    for (i = 0; i < src->used; i++)
        input[dst->used + i] = src->centroids[i];
    return consolidate(dst, input, dst->used + src->used,
                       dst->count + src->count, dst->compression);
}

int td_recompress(struct td_td *s, unsigned delta)
{
    struct td_centroid input[128];
    unsigned i;
    if (!td_valid(s) || delta < 8 || delta > s->compression)
        return TD_INVALID;
    for (i = 0; i < s->used; i++)
        input[i] = s->centroids[i];
    return consolidate(s, input, s->used, s->count, delta);
}

static double gap_mass(const struct td_centroid *a, const struct td_centroid *b)
{
    return (a->weight == 1 ? 0 : .5 * (double)a->weight) +
           (b->weight == 1 ? 0 : .5 * (double)b->weight);
}

int td_quantile(const struct td_td *s, double q, double *out)
{
    unsigned i;
    double rank, seen = 0;
    if (!td_valid(s) || !s->count || !(q >= 0 && q <= 1))
        return TD_INVALID;
    if (q == 0)
    {
        *out = s->centroids[TD_MUTANT == 2 ? s->used - 1 : 0].mean;
        return TD_OK;
    }
    if (q == 1)
    {
        *out = s->centroids[s->used - 1].mean;
        return TD_OK;
    }
    rank = q * (double)s->count;
    for (i = 0; i < s->used; i++)
    {
        const struct td_centroid *c = &s->centroids[i];
        if (c->weight == 1)
        {
            seen += 1;
            if (rank <= seen)
            {
                *out = c->mean;
                return TD_OK;
            }
        }
        if (i + 1 < s->used)
        {
            const struct td_centroid *next = &s->centroids[i + 1];
            double mass = gap_mass(c, next);
            if (mass && rank <= seen + mass)
            {
                double fraction = (rank - seen) / mass;
                *out = c->mean + (next->mean - c->mean) * fraction;
                return TD_OK;
            }
            seen += mass;
        }
    }
    *out = s->centroids[s->used - 1].mean;
    return TD_OK;
}

int td_cdf(const struct td_td *s, double x, double *out)
{
    unsigned i;
    double mass = 0;
    if (!td_valid(s) || !s->count || !(x >= -DBL_MAX && x <= DBL_MAX))
        return TD_INVALID;
    for (i = 0; i < s->used; i++)
    {
        const struct td_centroid *c = &s->centroids[i];
        if (c->weight == 1 && c->mean <= x)
            mass += 1;
        if (i + 1 < s->used)
        {
            const struct td_centroid *next = &s->centroids[i + 1];
            double gap = gap_mass(c, next);
            if (next->mean <= x)
                mass += gap;
            else if (c->mean < x)
                mass += gap * (x - c->mean) / (next->mean - c->mean);
        }
    }
    *out = mass / (double)s->count;
    return TD_OK;
}
