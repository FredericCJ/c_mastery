#include "outbox.h"

int rzs_open(struct rzj_journal *j, struct rzj_io io, struct msg_hub *h,
             struct rzs_outbox *o)
{
    (void)j;
    (void)io;
    (void)h;
    (void)o;
    return RZ_INVALID;
}

int rzs_apply(struct rzs_outbox *s, uint64_t n, struct rzj_request r, int *o,
              bool *d)
{
    (void)s;
    (void)n;
    (void)r;
    (void)o;
    (void)d;
    return RZ_INVALID;
}

int rzs_rotate(struct rzs_outbox *s, uint64_t n, uint64_t *o)
{
    (void)s;
    (void)n;
    (void)o;
    return RZ_INVALID;
}

int rzs_forward(struct rzs_outbox *s)
{
    (void)s;
    return RZ_INVALID;
}

int rzs_deliver(struct rzs_outbox *s, struct rzs_event *o)
{
    (void)s;
    (void)o;
    return RZ_INVALID;
}

int rzs_ack(struct rzs_outbox *s, unsigned n)
{
    (void)s;
    (void)n;
    return RZ_INVALID;
}
