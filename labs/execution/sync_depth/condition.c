#include "condition.h"
/* Complete the Mesa/state-ledger contract using the learner's CMX core. */
int cdq_init(CDQueue *q, cmx_kernel *k, unsigned n, unsigned b) {
  (void)q;
  (void)k;
  (void)n;
  (void)b;
  return CMX_UNIMPLEMENTED;
}
int cdq_begin(CDQueue *q, unsigned t, unsigned o, uint32_t v, cmx_deadline d) {
  (void)q;
  (void)t;
  (void)o;
  (void)v;
  (void)d;
  return CMX_UNIMPLEMENTED;
}
int cdq_begin_locked(CDQueue *q, unsigned t, unsigned o, uint32_t v,
                     cmx_deadline d) {
  (void)q;
  (void)t;
  (void)o;
  (void)v;
  (void)d;
  return CMX_UNIMPLEMENTED;
}
int cdq_resume(CDQueue *q, unsigned t) {
  (void)q;
  (void)t;
  return CMX_UNIMPLEMENTED;
}
int cdq_result(CDQueue *q, unsigned t, int *r, uint32_t *v) {
  (void)q;
  (void)t;
  (void)r;
  (void)v;
  return CMX_UNIMPLEMENTED;
}
int cdq_pulse(CDQueue *q, cmx_mutex *m, unsigned t, unsigned b) {
  (void)q;
  (void)m;
  (void)t;
  (void)b;
  return CMX_UNIMPLEMENTED;
}
int cdq_put_locked(CDQueue *q, unsigned t, uint32_t v) {
  (void)q;
  (void)t;
  (void)v;
  return CMX_UNIMPLEMENTED;
}
int cdq_get_locked(CDQueue *q, unsigned t, uint32_t *v) {
  (void)q;
  (void)t;
  (void)v;
  return CMX_UNIMPLEMENTED;
}
int cdq_shutdown(CDQueue *q, unsigned t) {
  (void)q;
  (void)t;
  return CMX_UNIMPLEMENTED;
}
int cdq_destroy(CDQueue *q) {
  (void)q;
  return CMX_UNIMPLEMENTED;
}
