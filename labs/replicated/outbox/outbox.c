#include "outbox.h"

int rpo_sender(struct rpl_log *e, struct rpl_log *a, struct rpo_sender *o)
{
    (void)e;
    (void)a;
    (void)o;
    return RPL_UNIMPLEMENTED;
}

int rpo_receiver(struct rpl_log *i, struct rpo_receiver *o)
{
    (void)i;
    (void)o;
    return RPL_UNIMPLEMENTED;
}

int rpo_next(struct rpo_sender *s, struct rpl_event *o)
{
    (void)s;
    (void)o;
    return RPL_UNIMPLEMENTED;
}

int rpo_receive(struct rpo_receiver *r, uint64_t u, uint32_t s,
                struct rpl_event e, struct rpo_reply *o)
{
    (void)r;
    (void)u;
    (void)s;
    (void)e;
    (void)o;
    return RPL_UNIMPLEMENTED;
}

int rpo_ack(struct rpo_sender *s, struct rpo_reply r)
{
    (void)s;
    (void)r;
    return RPL_UNIMPLEMENTED;
}

int rpo_projection(const struct rpl_log *e, unsigned p, int64_t *o)
{
    (void)e;
    (void)p;
    (void)o;
    return RPL_UNIMPLEMENTED;
}
