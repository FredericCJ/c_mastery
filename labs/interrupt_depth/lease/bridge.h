#ifndef CMASTERY_LEASE_BRIDGE_H
#define CMASTERY_LEASE_BRIDGE_H
#include "dma.h"
#include "lease.h"
/* Single serialized owner, same transfer length/generation, no other device
 * touching its extents. This is a policy adapter, not a second DMA driver. */
int xd_pd_cancel(struct xd_lease *, struct xd_ticket, struct pd_transfer *,
                 struct pd_io);
int xd_pd_finish(struct xd_lease *, struct xd_ticket, struct pd_transfer *,
                 struct pd_io);
#endif
