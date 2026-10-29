#include "sketch.h"

/* Learner task: implement rank registers, estimator and precision transfer. */
int sk_hll_init(struct sk_hll *s, unsigned p, uint32_t id)
{
    (void)s;
    (void)p;
    (void)id;
    return SK_INVALID;
}

int sk_hll_add(struct sk_hll *s, uint32_t h)
{
    (void)s;
    (void)h;
    return SK_INVALID;
}

int sk_hll_estimate(const struct sk_hll *s, double *v)
{
    (void)s;
    (void)v;
    return SK_INVALID;
}

int sk_hll_merge(struct sk_hll *s, const struct sk_hll *t)
{
    (void)s;
    (void)t;
    return SK_INVALID;
}

int sk_hll_fold(struct sk_hll *s, const struct sk_hll *t)
{
    (void)s;
    (void)t;
    return SK_INVALID;
}
