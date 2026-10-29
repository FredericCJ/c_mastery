#ifndef CMASTERY_SEMAPHORE_DEPTH_H
#define CMASTERY_SEMAPHORE_DEPTH_H
#include "wait.h"
struct cd_sem {
  struct cw_world wait;
  unsigned initial, priority_order, closed;
};
struct cd_post {
  unsigned scanned, eligible, woke, defer_schedule;
};
/* This interrupt mailbox is protected by a saved interrupt mask. Volatile is
 * visibility to that single-core interrupt model, not C thread synchronization.
 */
struct cd_deferred {
  volatile unsigned pending;
  unsigned maximum, closed;
  uint64_t accepted, drained, cancelled;
};
int cds_init(struct cd_sem *, cmx_kernel *, unsigned initial, unsigned maximum,
             unsigned priority_order);
int cds_take(struct cd_sem *, unsigned task, cmx_deadline, struct cw_token *);
int cds_post(struct cd_sem *, struct cd_post *);
int cds_close(struct cd_sem *);
int cds_deferred_init(struct cd_deferred *, unsigned maximum);
int cds_deferred_offer(struct cd_deferred *);
int cds_deferred_drain(struct cd_deferred *, struct cd_sem *, unsigned budget,
                       unsigned *drained);
int cds_deferred_close(struct cd_deferred *);
#endif
