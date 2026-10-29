#include "mailbox.h"

/* Intentional learner scaffold. Implement the transport/application boundary.
 */
int cdmb_init(struct cd_mailbox *m, cmx_kernel *k, unsigned c, uint32_t s)
{
    (void)m;
    (void)k;
    (void)c;
    (void)s;
    return CMX_UNIMPLEMENTED;
}

int cdmb_begin(struct cd_mailbox *m, unsigned s, const unsigned char r[4],
               cmx_deadline d, struct cdmb_identity *i)
{
    (void)m;
    (void)s;
    (void)r;
    (void)d;
    (void)i;
    return CMX_UNIMPLEMENTED;
}

int cdmb_delivery(struct cd_mailbox *m, unsigned s)
{
    (void)m;
    (void)s;
    return CMX_UNIMPLEMENTED;
}

int cdmb_receive(struct cd_mailbox *m, struct cdmb_request *r, unsigned c,
                 cmx_deadline d)
{
    (void)m;
    (void)r;
    (void)c;
    (void)d;
    return CMX_UNIMPLEMENTED;
}

int cdmb_reply(struct cd_mailbox *m, struct cdmb_identity i,
               const unsigned char r[4])
{
    (void)m;
    (void)i;
    (void)r;
    return CMX_UNIMPLEMENTED;
}

int cdmb_poll(struct cd_mailbox *m, unsigned s, unsigned char *r, unsigned c)
{
    (void)m;
    (void)s;
    (void)r;
    (void)c;
    return CMX_UNIMPLEMENTED;
}

int cdmb_cancel(struct cd_mailbox *m, unsigned s)
{
    (void)m;
    (void)s;
    return CMX_UNIMPLEMENTED;
}

int cdmb_restart(struct cd_mailbox *m)
{
    (void)m;
    return CMX_UNIMPLEMENTED;
}

int cdmb_effect(struct cd_mailbox *m, struct cdmb_identity i, uint32_t d,
                uint32_t *r)
{
    (void)m;
    (void)i;
    (void)d;
    (void)r;
    return CMX_UNIMPLEMENTED;
}

int cdmb_acknowledge(struct cd_mailbox *m, struct cdmb_identity i)
{
    (void)m;
    (void)i;
    return CMX_UNIMPLEMENTED;
}
