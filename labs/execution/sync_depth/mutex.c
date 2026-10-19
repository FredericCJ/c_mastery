#include "mutex.h"
/* Implement the complete contract; these compiling stubs are not solutions. */
int cdm_recursive_init(CDRecursive *r, cmx_kernel *k, unsigned maximum) {
  (void)r;
  (void)k;
  (void)maximum;
  return CMX_UNIMPLEMENTED;
}
int cdm_recursive_lock(CDRecursive *r, unsigned task, cmx_deadline d) {
  (void)r;
  (void)task;
  (void)d;
  return CMX_UNIMPLEMENTED;
}
int cdm_recursive_unlock(CDRecursive *r, unsigned task) {
  (void)r;
  (void)task;
  return CMX_UNIMPLEMENTED;
}
int cdm_recursive_close(CDRecursive *r) {
  (void)r;
  return CMX_UNIMPLEMENTED;
}
int cdm_ordered_lock(cmx_kernel *k, cmx_mutex *const *locks,
                     const unsigned *ranks, unsigned count, unsigned target,
                     unsigned task, cmx_deadline d) {
  (void)k;
  (void)locks;
  (void)ranks;
  (void)count;
  (void)target;
  (void)task;
  (void)d;
  return CMX_UNIMPLEMENTED;
}
void cdm_record_init(CDRecord *r, int32_t balance) {
  (void)r;
  (void)balance;
}
int cdm_record_read(cmx_kernel *k, cmx_mutex *m, unsigned task,
                    const CDRecord *r, CDRecord *out) {
  (void)k;
  (void)m;
  (void)task;
  (void)r;
  (void)out;
  return CMX_UNIMPLEMENTED;
}
int cdm_record_apply(cmx_kernel *k, cmx_mutex *m, unsigned task, CDRecord *r,
                     int32_t delta, CDRecordObserver observer, void *context) {
  (void)k;
  (void)m;
  (void)task;
  (void)r;
  (void)delta;
  (void)observer;
  (void)context;
  return CMX_UNIMPLEMENTED;
}
