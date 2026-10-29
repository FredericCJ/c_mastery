#ifndef CF_EPOCH_H
#define CF_EPOCH_H
#include "grace.h"
#include <stdatomic.h>

/* One C17 acquire/release gate serializes every ordinary AG field. Bounded
 * try calls return AG_BUSY while another owner holds it. A stalled owner can
 * prevent successful service; this is not a lock-free epoch algorithm. */
struct cf_epoch
{
    atomic_flag gate;
    struct ag_domain domain;
};

void cf_epoch_init(struct cf_epoch *, unsigned nested);
int cf_epoch_register(struct cf_epoch *, struct ag_lease *);
int cf_epoch_enter(struct cf_epoch *, struct ag_lease);
int cf_epoch_leave(struct cf_epoch *, struct ag_lease);
int cf_epoch_depart(struct cf_epoch *, struct ag_lease);
int cf_epoch_advance(struct cf_epoch *);
int cf_epoch_eligible(struct cf_epoch *, unsigned retired, unsigned *eligible);
#endif
