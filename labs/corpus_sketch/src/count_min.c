#include "sketch.h"

/* Learner task: implement weighted counting and transactional partition merge. */
int sk_cm_init(struct sk_cm *s, const struct sk_cm_config *c)
{
    (void)s;
    (void)c;
    return SK_INVALID;
}

int sk_cm_add(struct sk_cm *s, uint32_t k, uint64_t n)
{
    (void)s;
    (void)k;
    (void)n;
    return SK_INVALID;
}

int sk_cm_query(const struct sk_cm *s, uint32_t k, uint64_t *n)
{
    (void)s;
    (void)k;
    (void)n;
    return SK_INVALID;
}

int sk_cm_merge(struct sk_cm *s, const struct sk_cm *t)
{
    (void)s;
    (void)t;
    return SK_INVALID;
}
