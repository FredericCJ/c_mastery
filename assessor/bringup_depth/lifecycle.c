#include "lifecycle.h"
#ifndef BU_LIFECYCLE_MUTANT
#define BU_LIFECYCLE_MUTANT 0
#endif
BuResult bu_drain_begin(BuDrain *d, unsigned n, unsigned limit) {
  if (!d || n > 4 || !limit)
    return BU_INVALID;
  *d = (BuDrain){n, limit, 1, 0, 0, 0};
  return BU_OK;
}
BuResult bu_drain_step(BuDrain *d, unsigned ack, unsigned done) {
  if (!d || !d->stopped || d->transferred || ack > 1 || done > d->callbacks)
    return BU_INVALID;
  if (d->expired)
    return BU_TIMEOUT;
  d->callbacks -= done;
  if (ack)
    d->acknowledged = 1;
  if (d->acknowledged && (BU_LIFECYCLE_MUTANT == 1 || !d->callbacks)) {
    d->transferred = 1;
    return BU_OK;
  }
  if (!--d->remaining) {
    d->expired = 1;
    return BU_TIMEOUT;
  }
  return BU_FAILED; /* Still waiting; old owner retains all storage. */
}
BuResult bu_task_start(BuTask *t) {
  if (!t || (t->phase != 0 && BU_LIFECYCLE_MUTANT != 2) ||
      t->generation == UINT_MAX)
    return BU_INVALID;
  ++t->generation;
  t->phase = 1;
  t->pending = 0;
  for (unsigned i = 0; i < 8; ++i)
    t->task_bytes[i] = 0;
  return BU_OK;
}
BuResult bu_task_return(BuTask *t, unsigned g, int result, unsigned n) {
  if (!t || t->phase != 1 || g != t->generation || n > 4)
    return BU_INVALID;
  t->last_result = result;
  t->pending = n;
  t->phase = n ? 2 : 0;
  return BU_OK;
}
BuResult bu_task_drained(BuTask *t, unsigned g) {
  if (!t || t->phase != 2 || !t->pending ||
      (g != t->generation && BU_LIFECYCLE_MUTANT != 3))
    return BU_INVALID;
  if (!--t->pending)
    t->phase = 0;
  return BU_OK;
}
