#ifndef CMASTERY_INTERRUPT_LEASE_H
#define CMASTERY_INTERRUPT_LEASE_H
#include <stdint.h>
enum {
  XD_OK,
  XD_INVALID,
  XD_BUSY,
  XD_STALE,
  XD_DUPLICATE,
  XD_EXHAUSTED,
  XD_UNIMPLEMENTED = 99
};
enum { XD_QUARANTINE, XD_CPU, XD_DEVICE, XD_STOPPING, XD_SERVICE };
enum { XD_NONE, XD_COMPLETE, XD_STOPPED, XD_ERROR };
struct xd_ticket {
  uint32_t epoch, generation;
};
struct xd_lease {
  uint32_t epoch, generation, phase, terminal, terminal_count, length, complete,
      possible, quiescent;
};
/* Trusted supervisor observation from OUTSIDE a reset software domain.
 * Numeric fields cannot authenticate this statement; the port must prove it. */
struct xd_fence {
  uint32_t old_epoch, new_epoch, stopped, writes_drained;
};
struct xd_proof {
  struct xd_ticket ticket;
  uint32_t enabled, writes_pending, visibility_done, complete, possible;
};
struct xd_result {
  struct xd_ticket ticket;
  uint32_t terminal, complete, possible;
};
/* Zero-initialized state is quarantined, never immediately CPU-owned. */
int xd_recover(struct xd_lease *, const struct xd_fence *);
int xd_submit(struct xd_lease *, uint32_t length, struct xd_ticket *);
int xd_cancel(struct xd_lease *, struct xd_ticket);
int xd_observe(struct xd_lease *, struct xd_ticket, unsigned event, const struct xd_proof *);
int xd_reclaim(struct xd_lease *, struct xd_ticket, struct xd_result *);
void xd_quarantine(struct xd_lease *);
#endif
