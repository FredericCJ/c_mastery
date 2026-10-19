#include "reclamation.h"
int ap_init(APQueue *q, unsigned p, unsigned limit, APHook h, void *c) {
  (void)q;
  (void)p;
  (void)limit;
  (void)h;
  (void)c;
  return AP_UNIMPLEMENTED;
}
int ap_register(APQueue *q, APLease *out) {
  (void)q;
  (void)out;
  return AP_UNIMPLEMENTED;
}
int ap_leave(APQueue *q, APLease lease) {
  (void)q;
  (void)lease;
  return AP_UNIMPLEMENTED;
}
int ap_push(APQueue *q, APLease lease, APValue v) {
  (void)q;
  (void)lease;
  (void)v;
  return AP_UNIMPLEMENTED;
}
int ap_pop(APQueue *q, APLease lease, APValue *v) {
  (void)q;
  (void)lease;
  (void)v;
  return AP_UNIMPLEMENTED;
}
unsigned ap_collect(APQueue *q) {
  (void)q;
  return 0;
}
