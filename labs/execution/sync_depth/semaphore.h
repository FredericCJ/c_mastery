#ifndef CMASTERY_SEMAPHORE_DEPTH_H
#define CMASTERY_SEMAPHORE_DEPTH_H
#include "wait.h"
typedef struct {
  CWWorld wait;
  unsigned initial, priority_order, closed;
} CDSem;
typedef struct {
  unsigned scanned, eligible, woke, defer_schedule;
} CDPost;
/* This interrupt mailbox is protected by a saved interrupt mask. Volatile is
 * visibility to that single-core interrupt model, not C thread synchronization.
 */
typedef struct {
  volatile unsigned pending;
  unsigned maximum, closed;
  uint64_t accepted, drained, cancelled;
} CDDeferred;
int cds_init(CDSem *, cmx_kernel *, unsigned initial, unsigned maximum,
             unsigned priority_order);
int cds_take(CDSem *, unsigned task, cmx_deadline, CWToken *);
int cds_post(CDSem *, CDPost *);
int cds_close(CDSem *);
int cds_deferred_init(CDDeferred *, unsigned maximum);
int cds_deferred_offer(CDDeferred *);
int cds_deferred_drain(CDDeferred *, CDSem *, unsigned budget,
                       unsigned *drained);
int cds_deferred_close(CDDeferred *);
#endif
