/* Learner scaffold. Derive the collection invariants before filling these in. */
#include "collection.h"

int tc_init(struct tc_collection *q, enum tc_kind k, struct td_timer *t,
            size_t c, size_t *i, size_t *p, size_t *n, size_t *v, size_t *h,
            size_t b, uint64_t u, unsigned g)
{
    (void)q;
    (void)k;
    (void)t;
    (void)c;
    (void)i;
    (void)p;
    (void)n;
    (void)v;
    (void)h;
    (void)b;
    (void)u;
    (void)g;
    return -1;
}

int tc_arm(struct tc_collection *q, size_t s, uint64_t d, uint64_t p, int k,
           struct tc_token *t)
{
    (void)q;
    (void)s;
    (void)d;
    (void)p;
    (void)k;
    (void)t;
    return -1;
}

int tc_cancel(struct tc_collection *q, struct tc_token t)
{
    (void)q;
    (void)t;
    return -1;
}

int tc_peek(struct tc_collection *q, struct tc_token *t, uint64_t *d)
{
    (void)q;
    (void)t;
    (void)d;
    return -1;
}

int tc_begin(struct tc_collection *q, uint64_t n, struct tc_token *t,
             struct td_dispatch *d)
{
    (void)q;
    (void)n;
    (void)t;
    (void)d;
    return -1;
}

int tc_finish(struct tc_collection *q, struct tc_token t)
{
    (void)q;
    (void)t;
    return -1;
}
