#include "wakeup.h"

int sw_init(struct sw_world *w, unsigned n, unsigned c, struct sw_port p)
{
    (void)w;
    (void)n;
    (void)c;
    (void)p;
    return SQ_INVALID;
}

int sw_create(struct sw_world *w, unsigned a, unsigned h, unsigned x,
              struct sq_handle *i, struct sq_event *e)
{
    (void)w;
    (void)a;
    (void)h;
    (void)x;
    (void)i;
    (void)e;
    return SQ_INVALID;
}

int sw_wake(struct sw_world *w, unsigned a, unsigned h, struct sq_handle i,
            struct sq_event *e, struct sl_stats *s)
{
    (void)w;
    (void)a;
    (void)h;
    (void)i;
    (void)e;
    (void)s;
    return SQ_INVALID;
}

int sw_dispatch(struct sw_world *w, unsigned c, struct sq_handle *i,
                struct sq_event *e, struct sl_stats *s)
{
    (void)w;
    (void)c;
    (void)i;
    (void)e;
    (void)s;
    return SQ_INVALID;
}

int sw_finish(struct sw_world *w, unsigned c, struct sq_handle i, unsigned n,
              struct sq_event *e, struct sl_stats *s)
{
    (void)w;
    (void)c;
    (void)i;
    (void)n;
    (void)e;
    (void)s;
    return SQ_INVALID;
}

int sw_transfer_begin(struct sw_world *w, unsigned a, unsigned x, unsigned y,
                      struct sq_handle i, struct sq_transfer *t,
                      struct sq_event *e, struct sl_stats *s)
{
    (void)w;
    (void)a;
    (void)x;
    (void)y;
    (void)i;
    (void)t;
    (void)e;
    (void)s;
    return SQ_INVALID;
}

int sw_transfer_end(struct sw_world *w, unsigned a, struct sq_transfer t, int c,
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

int sw_retry(struct sw_world *w, unsigned a, unsigned d, struct sl_stats *s)
{
    (void)w;
    (void)a;
    (void)d;
    (void)s;
    return SQ_INVALID;
}

int sw_prepare_idle(struct sw_world *w, unsigned c, int m, struct sl_stats *s)
{
    (void)w;
    (void)c;
    (void)m;
    (void)s;
    return SQ_INVALID;
}
