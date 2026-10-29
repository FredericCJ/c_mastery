#include "durable.h"

/* Compile-ready learner scaffold. Implement the complete DURABLE.md contract.
 */
int rpl_pages(struct rpl_pages *a, struct rpl_bytes b, store_io *o)
{
    (void)a;
    (void)b;
    (void)o;
    return RPL_UNIMPLEMENTED;
}

int rpl_create(store *p, uint64_t u, uint32_t s, struct rpl_log *o)
{
    (void)p;
    (void)u;
    (void)s;
    (void)o;
    return RPL_UNIMPLEMENTED;
}

int rpl_open(store *p, uint64_t u, uint32_t s, struct rpl_log *o)
{
    (void)p;
    (void)u;
    (void)s;
    (void)o;
    return RPL_UNIMPLEMENTED;
}

int rpl_append(struct rpl_log *l, unsigned k, unsigned key, int32_t v)
{
    (void)l;
    (void)k;
    (void)key;
    (void)v;
    return RPL_UNIMPLEMENTED;
}

int rpl_get(const struct rpl_log *l, unsigned p, unsigned k, int32_t *o)
{
    (void)l;
    (void)p;
    (void)k;
    (void)o;
    return RPL_UNIMPLEMENTED;
}

int rpl_event_encode(uint64_t u, uint32_t s, struct rpl_event e,
                     unsigned char o[64])
{
    (void)u;
    (void)s;
    (void)e;
    (void)o;
    return RPL_UNIMPLEMENTED;
}

int rpl_event_decode(uint64_t u, uint32_t s, const unsigned char i[64],
                     struct rpl_event *o)
{
    (void)u;
    (void)s;
    (void)i;
    (void)o;
    return RPL_UNIMPLEMENTED;
}

int rpl_snapshot_save(struct rpl_log *l, unsigned p)
{
    (void)l;
    (void)p;
    return RPL_UNIMPLEMENTED;
}

int rpl_snapshot_load(struct rpl_log *l, struct rpl_snapshot *o)
{
    (void)l;
    (void)o;
    return RPL_UNIMPLEMENTED;
}

int rpl_state(struct rpl_log *l, unsigned u, struct rpl_snapshot *o)
{
    (void)l;
    (void)u;
    (void)o;
    return RPL_UNIMPLEMENTED;
}
