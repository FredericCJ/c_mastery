#include "index.h"
#ifndef CI_MUTANT
#define CI_MUTANT 0
#endif
static uint32_t mul(uint32_t a, uint32_t b)
{
    return (uint32_t)(((uint64_t)a * b) % CI_MOD);
}

static struct ci_aggregate identity(void)
{
    struct ci_aggregate x = {0, 1, 0, 0};
    return x;
}

static struct ci_aggregate join(struct ci_aggregate l, struct ci_aggregate r)
{
    if (CI_MUTANT == 3)
    {
        struct ci_aggregate t = l;
        l = r;
        r = t;
    }
    struct ci_aggregate o;
    o.hash = (mul(l.hash, r.power) + r.hash) % CI_MOD;
    o.power = mul(l.power, r.power);
    o.geometric = (mul(l.geometric, r.power) + r.geometric) % CI_MOD;
    o.length = l.length + r.length;
    return o;
}

static void map(struct ci_segment_node *p, uint32_t a, uint32_t b)
{
    p->value.hash =
        (mul(a, p->value.hash) + mul(b, p->value.geometric)) % CI_MOD;
    p->b = CI_MUTANT == 4 ? (mul(p->a, b) + p->b) % CI_MOD
                          : (mul(a, p->b) + b) % CI_MOD;
    p->a = mul(a, p->a);
}

static void build(struct ci_segment *s, unsigned p, unsigned lo, unsigned hi,
                  const uint32_t *v)
{
    s->node[p].a = 1;
    s->node[p].b = 0;
    if (hi - lo == 1)
    {
        struct ci_aggregate x = {v[lo], 257, 1, 1};
        s->node[p].value = x;
        return;
    }
    unsigned m = lo + (hi - lo) / 2;
    build(s, p * 2, lo, m, v);
    build(s, p * 2 + 1, m, hi, v);
    s->node[p].value = join(s->node[p * 2].value, s->node[p * 2 + 1].value);
}

enum ci_status ci_segment_init(struct ci_segment *s, const uint32_t *v,
                               unsigned n)
{
    if (!s || (!v && n))
        return CI_INVALID;
    if (n > CI_CAP)
        return CI_RANGE;
    for (unsigned i = 0; i < n; i++)
        if (v[i] >= CI_MOD)
            return CI_RANGE;
    s->n = n;
    for (unsigned i = 0; i < 4 * CI_CAP; i++)
    {
        s->node[i].value = identity();
        s->node[i].a = 1;
        s->node[i].b = 0;
    }
    if (n)
        build(s, 1, 0, n, v);
    return CI_OK;
}

static void update(struct ci_segment *s, unsigned p, unsigned lo, unsigned hi,
                   unsigned l, unsigned r, uint32_t a, uint32_t b)
{
    if (l <= lo && hi <= r)
    {
        map(&s->node[p], a, b);
        return;
    }
    unsigned m = lo + (hi - lo) / 2;
    map(&s->node[p * 2], s->node[p].a, s->node[p].b);
    map(&s->node[p * 2 + 1], s->node[p].a, s->node[p].b);
    s->node[p].a = 1;
    s->node[p].b = 0;
    if (l < m)
        update(s, p * 2, lo, m, l, r, a, b);
    if (r > m)
        update(s, p * 2 + 1, m, hi, l, r, a, b);
    s->node[p].value = join(s->node[p * 2].value, s->node[p * 2 + 1].value);
}

enum ci_status ci_segment_apply(struct ci_segment *s, unsigned l, unsigned r,
                                uint32_t a, uint32_t b)
{
    if (!s)
        return CI_INVALID;
    if (l > r || r > s->n || a >= CI_MOD || b >= CI_MOD)
        return CI_RANGE;
    if (l < r)
        update(s, 1, 0, s->n, l, r, a, b);
    return CI_OK;
}

static struct ci_aggregate query(const struct ci_segment *s, unsigned p,
                                 unsigned lo, unsigned hi, unsigned l,
                                 unsigned r, uint32_t a, uint32_t b)
{
    if (l <= lo && hi <= r)
    {
        struct ci_aggregate x = s->node[p].value;
        x.hash = (mul(a, x.hash) + mul(b, x.geometric)) % CI_MOD;
        return x;
    }
    uint32_t nb = (mul(a, s->node[p].b) + b) % CI_MOD,
             na = mul(a, s->node[p].a);
    unsigned m = lo + (hi - lo) / 2;
    struct ci_aggregate x = identity(), y = identity();
    if (l < m)
        x = query(s, p * 2, lo, m, l, r, na, nb);
    if (r > m)
        y = query(s, p * 2 + 1, m, hi, l, r, na, nb);
    return join(x, y);
}

enum ci_status ci_segment_query(const struct ci_segment *s, unsigned l,
                                unsigned r, struct ci_aggregate *out)
{
    if (!s || !out)
        return CI_INVALID;
    if (l > r || r > s->n)
        return CI_RANGE;
    *out = l == r ? identity() : query(s, 1, 0, s->n, l, r, 1, 0);
    return CI_OK;
}
