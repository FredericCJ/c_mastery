#ifndef CF_EPOCH_H
#define CF_EPOCH_H
#include "grace.h"
#include <stdatomic.h>
/* One C17 acquire/release gate serializes every ordinary AG field. Bounded
 * try calls return AG_BUSY while another owner holds it. A stalled owner can
 * prevent successful service; this is not a lock-free epoch algorithm. */
typedef struct {
  atomic_flag gate;
  AGDomain domain;
} CfEpoch;
void cf_epoch_init(CfEpoch *, unsigned nested);
int cf_epoch_register(CfEpoch *, AGLease *);
int cf_epoch_enter(CfEpoch *, AGLease);
int cf_epoch_leave(CfEpoch *, AGLease);
int cf_epoch_depart(CfEpoch *, AGLease);
int cf_epoch_advance(CfEpoch *);
int cf_epoch_eligible(CfEpoch *, unsigned retired, unsigned *eligible);
#endif
