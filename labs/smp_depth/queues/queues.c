#include "queues.h"

int sq_init(struct sq_world *w, unsigned n, unsigned c, unsigned l)
{
    (void)w;
    (void)n;
    (void)c;
    (void)l;
    return SQ_INVALID;
}

int sq_create(struct sq_world *w, unsigned a, unsigned q, unsigned s,
              struct sq_handle *h, struct sq_event *e)
{
    (void)w;
    (void)a;
    (void)q;
    (void)s;
    (void)h;
    (void)e;
    return SQ_INVALID;
}

int sq_dispatch(struct sq_world *w, unsigned c, struct sq_handle *h,
                struct sq_event *e, struct sl_stats *s)
{
    (void)w;
    (void)c;
    (void)h;
    (void)e;
    (void)s;
    return SQ_INVALID;
}

int sq_finish(struct sq_world *w, unsigned c, struct sq_handle h, unsigned n,
              struct sq_event *e, struct sl_stats *s)
{
    (void)w;
    (void)c;
    (void)h;
    (void)n;
    (void)e;
    (void)s;
    return SQ_INVALID;
}

int sq_wake(struct sq_world *w, unsigned a, unsigned q, struct sq_handle h,
            struct sq_event *e, struct sl_stats *s)
{
    (void)w;
    (void)a;
    (void)q;
    (void)h;
    (void)e;
    (void)s;
    return SQ_INVALID;
}

int sq_transfer_begin(struct sq_world *w, unsigned a, unsigned x, unsigned y,
                      struct sq_handle h, struct sq_transfer *t,
                      struct sq_event *e, struct sl_stats *s)
{
    (void)w;
    (void)a;
    (void)x;
    (void)y;
    (void)h;
    (void)t;
    (void)e;
    (void)s;
    return SQ_INVALID;
}

int sq_transfer_end(struct sq_world *w, unsigned a, struct sq_transfer t, int c,
                    struct sq_event *e, struct sl_stats *s)
{
    (void)w;
    (void)a;
    (void)t;
    (void)c;
    (void)e;
    (void)s;
    return SQ_INVALID;
}

int sq_invariant(const struct sq_world *w)
{
    (void)w;
    return 0;
}
