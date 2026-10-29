#include "sketch.h"
#include <float.h>
#ifndef SK_MUTANT
#define SK_MUTANT 0
#endif
_Static_assert(DBL_MANT_DIG >= 53 && DBL_MAX_EXP >= 1024,
               "53-bit double required");

static int valid(const struct sk_hll *s)
{
    unsigned i;
    if (s->p < 4 || s->p > 8)
    {
        return 0;
    }
    for (i = 0; i < (1u << s->p); i++)
    {
        if (s->reg[i] > 33u - s->p)
        {
            return 0;
        }
    }
    return 1;
}

/* Positive bounded arguments only. Range reduction then atanh series, |z|<=1/3.
   24 terms give truncation < 2*(1/3)^49/(49*(1-1/9)); rounding is separate. */
static double logarithm(double x)
{
    int exponent = 0;
    unsigned i;
    double z, term, total;
    while (x >= 2.0)
    {
        x *= 0.5;
        exponent++;
    }
    while (x < 1.0)
    {
        x *= 2.0;
        exponent--;
    }
    z = (x - 1.0) / (x + 1.0);
    term = z;
    total = 0.0;
    for (i = 0; i < 24; i++)
    {
        total += term / (double)(2u * i + 1u);
        term *= z * z;
    }
    return 2.0 * total + (double)exponent * 0.69314718055994530942;
}

int sk_hll_init(struct sk_hll *s, unsigned p, uint32_t identity)
{
    struct sk_hll next = {0};
    if (p < 4 || p > 8)
    {
        return SK_INVALID;
    }
    next.p = p;
    next.identity = identity;
    *s = next;
    return SK_OK;
}

int sk_hll_add(struct sk_hll *s, uint32_t hash)
{
    unsigned index, rank = 1, q;
    uint32_t bit;
    if (!valid(s))
    {
        return SK_INVALID;
    }
    q = 32u - s->p;
    index = (unsigned)(hash >> q);
    bit = UINT32_C(1) << (q - 1u);
    while (bit && !(hash & bit))
    {
        rank++;
        bit >>= 1;
    }
    if (SK_MUTANT == 1)
    {
        rank = 1;
    }
    if (rank > s->reg[index])
    {
        s->reg[index] = (uint8_t)rank;
    }
    return SK_OK;
}

int sk_hll_estimate(const struct sk_hll *s, double *out)
{
    unsigned i, j, m, zeros = 0;
    double sum = 0.0, alpha, value;
    if (!valid(s))
    {
        return SK_INVALID;
    }
    m = 1u << s->p;
    for (i = 0; i < m; i++)
    {
        double weight = 1.0;
        if (!s->reg[i])
        {
            zeros++;
        }
        for (j = 0; j < s->reg[i]; j++)
        {
            weight *= 0.5;
        }
        sum += weight;
    }
    alpha = m == 16   ? .673
            : m == 32 ? .697
            : m == 64 ? .709
                      : .7213 / (1.0 + 1.079 / (double)m);
    value = alpha * (double)m * (double)m / sum;
    if (value <= 2.5 * (double)m && zeros)
    {
        value = (double)m * logarithm((double)m / (double)zeros);
    }
    else if (value > 4294967296.0 / 30.0)
    {
        if (value >= 4294967296.0)
        {
            return SK_OVERFLOW;
        }
        value = -4294967296.0 * logarithm(1.0 - value / 4294967296.0);
    }
    *out = value;
    return SK_OK;
}

int sk_hll_fold(struct sk_hll *dst, const struct sk_hll *src)
{
    unsigned i, d, mask;
    if (!valid(dst) || !valid(src) || dst->identity != src->identity ||
        dst->p > src->p)
    {
        return SK_INVALID;
    }
    d = src->p - dst->p;
    mask = (1u << d) - 1u;
    for (i = 0; i < (1u << src->p); i++)
    {
        if (src->reg[i])
        {
            unsigned rank, j = i >> d, suffix = i & mask;
            if (!suffix)
            {
                rank = (unsigned)src->reg[i] + d;
            }
            else
            {
                unsigned bit = 1u << (d - 1u);
                rank = 1;
                while (!(suffix & bit))
                {
                    rank++;
                    bit >>= 1;
                }
            }
            if (SK_MUTANT == 2)
            {
                rank = src->reg[i];
            }
            if (rank > dst->reg[j])
            {
                dst->reg[j] = (uint8_t)rank;
            }
        }
    }
    return SK_OK;
}

int sk_hll_merge(struct sk_hll *dst, const struct sk_hll *src)
{
    if (dst->p != src->p)
    {
        return SK_INVALID;
    }
    return sk_hll_fold(dst, src);
}
