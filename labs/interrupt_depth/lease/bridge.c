#include "bridge.h"

int xd_pd_cancel(struct xd_lease *s, struct xd_ticket t, struct pd_transfer *p,
                 struct pd_io io)
{
    (void)s;
    (void)t;
    (void)p;
    (void)io;
    return XD_UNIMPLEMENTED;
}

int xd_pd_finish(struct xd_lease *s, struct xd_ticket t, struct pd_transfer *p,
                 struct pd_io io)
{
    (void)s;
    (void)t;
    (void)p;
    (void)io;
    return XD_UNIMPLEMENTED;
}
