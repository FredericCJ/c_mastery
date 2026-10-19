#include "wait.h"
/* Deliberately unsolved wrapper: complete the generational CMX wait contract.
 */
void cw_init(CWWorld *w, cmx_kernel *k) {
  (void)w;
  (void)k;
}
int cw_object_init(CWWorld *w, unsigned i, unsigned t, unsigned n, unsigned m) {
  (void)w;
  (void)i;
  (void)t;
  (void)n;
  (void)m;
  return CMX_UNIMPLEMENTED;
}
int cw_begin(CWWorld *w, unsigned i, unsigned t, cmx_deadline d, unsigned s,
             CWToken *o) {
  (void)w;
  (void)i;
  (void)t;
  (void)d;
  (void)s;
  (void)o;
  return CMX_UNIMPLEMENTED;
}
int cw_result(CWToken t, int *r, unsigned *c) {
  (void)t;
  (void)r;
  (void)c;
  return CMX_UNIMPLEMENTED;
}
int cw_post(CWWorld *w, unsigned i) {
  (void)w;
  (void)i;
  return CMX_UNIMPLEMENTED;
}
int cw_ready_callback(CWToken t) {
  (void)t;
  return CMX_UNIMPLEMENTED;
}
int cw_timeout(CWToken t, uint32_t n) {
  (void)t;
  (void)n;
  return CMX_UNIMPLEMENTED;
}
int cw_cancel(CWToken t) {
  (void)t;
  return CMX_UNIMPLEMENTED;
}
int cw_unlock(CWWorld *w, unsigned i, unsigned t) {
  (void)w;
  (void)i;
  (void)t;
  return CMX_UNIMPLEMENTED;
}
int cw_close(CWWorld *w, unsigned i) {
  (void)w;
  (void)i;
  return CMX_UNIMPLEMENTED;
}
int cw_exit_reap(CWWorld *w, unsigned t) {
  (void)w;
  (void)t;
  return CMX_UNIMPLEMENTED;
}
