#include "index.h"
#ifndef CI_MUTANT
#define CI_MUTANT 0
#endif
static uint32_t sum(const struct ci_fenwick *f, unsigned n)
{
    uint32_t s = 0;
    while (n)
    {
        s += f->bit[n];
        n &= n - 1u;
    }
    return s;
}

enum ci_status ci_fenwick_init(struct ci_fenwick *f, const uint32_t *v,
                               unsigned n)
{
    if (!f || (!v && n))
    {
        return CI_INVALID;
    }
    if (n > CI_CAP)
    {
        return CI_RANGE;
    }
    uint64_t total = 0;
    for (unsigned i = 0; i < n; i++)
    {
        total += v[i];
    }
    if (total > UINT32_MAX)
    {
        return CI_RANGE;
    }
    struct ci_fenwick t;
    t.n = n;
    t.total = (uint32_t)total;
    for (unsigned i = 0; i <= CI_CAP; i++)
    {
        t.bit[i] = 0;
    }
    for (unsigned i = 1; i <= n; i++)
    {
        t.bit[i] += v[i - 1];
        unsigned j = i + (i & (~i + 1u));
        if (j <= n)
        {
            t.bit[j] += t.bit[i];
        }
    }
    *f = t;
    return CI_OK;
}

enum ci_status ci_fenwick_prefix(const struct ci_fenwick *f, unsigned n,
                                 uint32_t *out)
{
    if (!f || !out)
    {
        return CI_INVALID;
    }
    if (n > f->n)
    {
        return CI_RANGE;
    }
    *out = sum(f, n);
    return CI_OK;
}

enum ci_status ci_fenwick_add(struct ci_fenwick *f, unsigned at, int64_t delta)
{
    if (!f)
    {
        return CI_INVALID;
    }
    if (at >= f->n)
    {
        return CI_RANGE;
    }
    uint32_t old = sum(f, at + 1) - sum(f, at);
    if (delta < -(int64_t)old || delta > (int64_t)UINT32_MAX - f->total)
    {
        return CI_RANGE;
    }
    for (unsigned i = at + 1; i <= f->n; i += i & (~i + 1u))
    {
        f->bit[i] = (uint32_t)((int64_t)f->bit[i] + delta);
    }
    f->total = (uint32_t)((int64_t)f->total + delta);
    return CI_OK;
}

enum ci_status ci_fenwick_select(const struct ci_fenwick *f, uint32_t rank,
                                 unsigned *out)
{
    if (!f || !out)
    {
        return CI_INVALID;
    }
    if (rank >= f->total)
    {
        return CI_RANGE;
    }
    unsigned at = 0, step = 1;
    while (step <= f->n / 2)
    {
        step *= 2;
    }
    for (; step; step /= 2)
    {
        unsigned next = at + step;
        if (next <= f->n &&
            (CI_MUTANT == 1 ? f->bit[next] < rank : f->bit[next] <= rank))
        {
            rank -= f->bit[next];
            at = next;
        }
    }
    *out = at;
    return CI_OK;
}

enum ci_status ci_fenwick_rescale(struct ci_fenwick *f, unsigned keep_positive)
{
    if (!f)
    {
        return CI_INVALID;
    }
    if (keep_positive > 1)
    {
        return CI_RANGE;
    }
    if (CI_MUTANT == 2)
    {
        for (unsigned i = 1; i <= f->n; i++)
        {
            f->bit[i] /= 2;
        }
        f->total = sum(f, f->n);
        return CI_OK;
    }
    uint32_t values[CI_CAP];
    for (unsigned i = 0; i < f->n; i++)
    {
        values[i] = f->bit[i + 1];
    }
    for (unsigned i = f->n; i; i--)
    {
        unsigned parent = i + (i & (~i + 1u));
        if (parent <= f->n)
        {
            values[parent - 1] -= values[i - 1];
        }
    }
    for (unsigned i = 0; i < f->n; i++)
    {
        uint32_t x = values[i];
        values[i] = x / 2 + (keep_positive && (x % 2));
    }
    return ci_fenwick_init(f, values, f->n);
}
