#include "core.h"

int cf_ring_init(struct cf_ring *q, unsigned n, unsigned l, unsigned b)
{
    (void)q;
    (void)n;
    (void)l;
    (void)b;
    return CF_UNIMPLEMENTED;
}

int cf_push(struct cf_ring *q, const struct cf_value *v, unsigned n,
            cf_hook_fn h, void *c)
{
    (void)q;
    (void)v;
    (void)n;
    (void)h;
    (void)c;
    return CF_UNIMPLEMENTED;
}

int cf_pop(struct cf_ring *q, struct cf_value *v, unsigned n, cf_hook_fn h,
           void *c)
{
    (void)q;
    (void)v;
    (void)n;
    (void)h;
    (void)c;
    return CF_UNIMPLEMENTED;
}

int cf_increment(atomic_uint *v, unsigned *o, unsigned *a, unsigned s)
{
    (void)v;
    (void)o;
    (void)a;
    (void)s;
    return CF_UNIMPLEMENTED;
}

int cf_pool_init(struct cf_pool *p, unsigned l)
{
    (void)p;
    (void)l;
    return CF_UNIMPLEMENTED;
}

int cf_take(struct cf_pool *p, unsigned w, unsigned s, struct cf_ticket *o,
            unsigned *n)
{
    (void)p;
    (void)w;
    (void)s;
    (void)o;
    (void)n;
    return CF_UNIMPLEMENTED;
}

int cf_publish(struct cf_pool *p, struct cf_ticket t)
{
    (void)p;
    (void)t;
    return CF_UNIMPLEMENTED;
}

int cf_cancel(struct cf_pool *p, struct cf_ticket t)
{
    (void)p;
    (void)t;
    return CF_UNIMPLEMENTED;
}

int cf_retire(struct cf_pool *p, struct cf_ticket t)
{
    (void)p;
    (void)t;
    return CF_UNIMPLEMENTED;
}

int cf_reclaim(struct cf_pool *p, struct cf_ticket t)
{
    (void)p;
    (void)t;
    return CF_UNIMPLEMENTED;
}
