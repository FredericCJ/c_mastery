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
typedef struct {
  uint32_t epoch, generation;
} XdTicket;
typedef struct {
  uint32_t epoch, generation, phase, terminal, terminal_count, length, complete,
      possible, quiescent;
} XdLease;
/* Trusted supervisor observation from OUTSIDE a reset software domain.
 * Numeric fields cannot authenticate this statement; the port must prove it. */
typedef struct {
  uint32_t old_epoch, new_epoch, stopped, writes_drained;
} XdFence;
typedef struct {
  XdTicket ticket;
  uint32_t enabled, writes_pending, visibility_done, complete, possible;
} XdProof;
typedef struct {
  XdTicket ticket;
  uint32_t terminal, complete, possible;
} XdResult;
/* Zero-initialized state is quarantined, never immediately CPU-owned. */
int xd_recover(XdLease *, const XdFence *);
int xd_submit(XdLease *, uint32_t length, XdTicket *);
int xd_cancel(XdLease *, XdTicket);
int xd_observe(XdLease *, XdTicket, unsigned event, const XdProof *);
int xd_reclaim(XdLease *, XdTicket, XdResult *);
void xd_quarantine(XdLease *);
#endif
