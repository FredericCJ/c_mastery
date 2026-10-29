#include "lease.h"

int xd_recover(struct xd_lease *s, const struct xd_fence *f)
{
    (void)s;
    (void)f;
    return XD_UNIMPLEMENTED;
}

int xd_submit(struct xd_lease *s, uint32_t n, struct xd_ticket *t)
{
    (void)s;
    (void)n;
    (void)t;
    return XD_UNIMPLEMENTED;
}

int xd_cancel(struct xd_lease *s, struct xd_ticket t)
{
    (void)s;
    (void)t;
    return XD_UNIMPLEMENTED;
}

int xd_observe(struct xd_lease *s, struct xd_ticket t, unsigned e,
               const struct xd_proof *p)
{
    (void)s;
    (void)t;
    (void)e;
    (void)p;
    return XD_UNIMPLEMENTED;
}

int xd_reclaim(struct xd_lease *s, struct xd_ticket t, struct xd_result *r)
{
    (void)s;
    (void)t;
    (void)r;
    return XD_UNIMPLEMENTED;
}

void xd_quarantine(struct xd_lease *s)
{
    (void)s;
}
