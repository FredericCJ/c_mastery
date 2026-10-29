#include "reclamation.h"
int ap_init(struct ap_queue *q, unsigned p, unsigned limit, ap_hook_fn h, void *c) {
  (void)q;
  (void)p;
  (void)limit;
  (void)h;
  (void)c;
  return AP_UNIMPLEMENTED;
}
int ap_register(struct ap_queue *q, struct ap_lease *out) {
  (void)q;
  (void)out;
  return AP_UNIMPLEMENTED;
}
int ap_leave(struct ap_queue *q, struct ap_lease lease) {
  (void)q;
  (void)lease;
  return AP_UNIMPLEMENTED;
}
int ap_push(struct ap_queue *q, struct ap_lease lease, struct ap_value v) {
  (void)q;
  (void)lease;
  (void)v;
  return AP_UNIMPLEMENTED;
}
int ap_pop(struct ap_queue *q, struct ap_lease lease, struct ap_value *v) {
  (void)q;
  (void)lease;
  (void)v;
  return AP_UNIMPLEMENTED;
}
unsigned ap_collect(struct ap_queue *q) {
  (void)q;
  return 0;
}
