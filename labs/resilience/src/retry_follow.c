#include "retry.h"

int rzr_reduce(uint32_t s, unsigned b, uint32_t n, uint32_t *o)
{
    (void)s;
    (void)b;
    (void)n;
    (void)o;
    return RZ_INVALID;
}

int rzr_uniform(struct rzr_random *r, uint32_t n, uint32_t *o)
{
    (void)r;
    (void)n;
    (void)o;
    return RZ_INVALID;
}

int rzr_init(struct rzr_retry *r, struct rzf_scope *s, struct rzf_bucket *b,
             struct net_stack *n, unsigned m, uint64_t a, uint64_t c, bool f,
             uint32_t d)
{
    (void)r;
    (void)s;
    (void)b;
    (void)n;
    (void)m;
    (void)a;
    (void)c;
    (void)f;
    (void)d;
    return RZ_INVALID;
}

int rzr_original(struct rzr_retry *r, uint64_t n, v4_packet_sink_fn s, void *c)
{
    (void)r;
    (void)n;
    (void)s;
    (void)c;
    return RZ_INVALID;
}

int rzr_plan(struct rzr_retry *r, uint64_t n, uint64_t a, enum rzr_jitter j,
             uint64_t *o)
{
    (void)r;
    (void)n;
    (void)a;
    (void)j;
    (void)o;
    return RZ_INVALID;
}

int rzr_dispatch(struct rzr_retry *r, uint64_t n, v4_packet_sink_fn s, void *c)
{
    (void)r;
    (void)n;
    (void)s;
    (void)c;
    return RZ_INVALID;
}

int rzr_ack_packet(struct rzr_retry *r, const void *b, size_t z, uint64_t n,
                   struct net_packet *o)
{
    (void)r;
    (void)b;
    (void)z;
    (void)n;
    (void)o;
    return RZ_INVALID;
}

int rzr_finish(struct rzr_retry *r)
{
    (void)r;
    return RZ_INVALID;
}

int rzr_abort(struct rzr_retry *r)
{
    (void)r;
    return RZ_INVALID;
}
