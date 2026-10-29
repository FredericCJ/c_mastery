#include "event_follow.h"

int pf_post(struct pf_hsm *h, unsigned e)
{
    (void)h;
    (void)e;
    return PT_UNIMPLEMENTED;
}

int pf_dispatch(struct pf_hsm *h, unsigned e, unsigned g, unsigned l,
                pf_entry_fn f, void *c, unsigned char *t, size_t n, size_t *u)
{
    (void)h;
    (void)e;
    (void)g;
    (void)l;
    (void)f;
    (void)c;
    (void)t;
    (void)n;
    (void)u;
    return PT_UNIMPLEMENTED;
}

int pf_active_post(struct pf_active *a, unsigned e)
{
    (void)a;
    (void)e;
    return PT_UNIMPLEMENTED;
}

int pf_active_step(struct pf_active *a, unsigned char *t, size_t n, size_t *u)
{
    (void)a;
    (void)t;
    (void)n;
    (void)u;
    return PT_UNIMPLEMENTED;
}

int pf_active_discard(struct pf_active *a, unsigned *e)
{
    (void)a;
    (void)e;
    return PT_UNIMPLEMENTED;
}

int pf_reactive_init(struct pf_reactive *b, const struct pf_node *p, size_t n)
{
    (void)b;
    (void)p;
    (void)n;
    return PT_UNIMPLEMENTED;
}

int pf_reactive_tick(struct pf_reactive *b, const struct pf_node *p, size_t n,
                     unsigned g, pf_leaf_fn l, pf_abort_fn a, void *c,
                     unsigned *o)
{
    (void)b;
    (void)p;
    (void)n;
    (void)g;
    (void)l;
    (void)a;
    (void)c;
    (void)o;
    return PT_UNIMPLEMENTED;
}

int pf_reactive_cancel(struct pf_reactive *b, pf_abort_fn a, void *c)
{
    (void)b;
    (void)a;
    (void)c;
    return PT_UNIMPLEMENTED;
}
