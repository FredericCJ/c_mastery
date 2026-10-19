#include "lifecycle.h"
BuResult bu_drain_begin(BuDrain *d, unsigned n, unsigned limit) {
  (void)d;
  (void)n;
  (void)limit;
  return BU_UNSUPPORTED;
}
BuResult bu_drain_step(BuDrain *d, unsigned ack, unsigned done) {
  (void)d;
  (void)ack;
  (void)done;
  return BU_UNSUPPORTED;
}
BuResult bu_task_start(BuTask *t) {
  (void)t;
  return BU_UNSUPPORTED;
}
BuResult bu_task_return(BuTask *t, unsigned g, int result, unsigned n) {
  (void)t;
  (void)g;
  (void)result;
  (void)n;
  return BU_UNSUPPORTED;
}
BuResult bu_task_drained(BuTask *t, unsigned g) {
  (void)t;
  (void)g;
  return BU_UNSUPPORTED;
}
