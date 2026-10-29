#include "query.h"

/* Intentionally unfinished learner source, separate from private reference. */
int qi_build(const struct qi_row *r, size_t n, struct qi_index *o)
{
    (void)r;
    (void)n;
    (void)o;
    return QI_INVALID;
}

int qi_replace(struct qi_index *x, size_t r, struct qi_row v, int l)
{
    (void)x;
    (void)r;
    (void)v;
    (void)l;
    return QI_INVALID;
}

int qi_rebuild_zones(struct qi_index *x)
{
    (void)x;
    return QI_INVALID;
}

int qi_terms(const struct qi_index *x, unsigned a, unsigned b, uint32_t *o)
{
    (void)x;
    (void)a;
    (void)b;
    (void)o;
    return QI_INVALID;
}

int qi_compare(const struct qi_index *x, int16_t t, struct qi_truth *o)
{
    (void)x;
    (void)t;
    (void)o;
    return QI_INVALID;
}

int qi_logic(const struct qi_index *x, struct qi_truth a, struct qi_truth b,
             unsigned p, struct qi_truth *o)
{
    (void)x;
    (void)a;
    (void)b;
    (void)p;
    (void)o;
    return QI_INVALID;
}

int qi_phrase(const struct qi_index *x, const uint8_t *q, size_t n, uint32_t *o)
{
    (void)x;
    (void)q;
    (void)n;
    (void)o;
    return QI_INVALID;
}

int qi_near(const struct qi_index *x, unsigned a, unsigned b, unsigned d,
            uint32_t *o)
{
    (void)x;
    (void)a;
    (void)b;
    (void)d;
    (void)o;
    return QI_INVALID;
}

int qi_range(const struct qi_index *x, int16_t a, int16_t b, uint32_t *o,
             struct qi_cost *c)
{
    (void)x;
    (void)a;
    (void)b;
    (void)o;
    (void)c;
    return QI_INVALID;
}

int qi_search(const struct qi_index *x, unsigned a, unsigned b, int16_t l,
              int16_t h, const uint8_t *p, size_t n, uint32_t *o,
              struct qi_cost *c)
{
    (void)x;
    (void)a;
    (void)b;
    (void)l;
    (void)h;
    (void)p;
    (void)n;
    (void)o;
    (void)c;
    return QI_INVALID;
}
