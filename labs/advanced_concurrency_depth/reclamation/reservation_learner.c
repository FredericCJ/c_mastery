#include "reservation.h"
void ar_init(struct ar_queue *q, ap_hook_fn h, void *c) {
  (void)q;
  (void)h;
  (void)c;
}
int ar_reserve(struct ar_queue *q, struct ar_ticket *t) {
  (void)q;
  (void)t;
  return AR_UNIMPLEMENTED;
}
int ar_finish(struct ar_queue *q, struct ar_ticket t, struct ap_value v) {
  (void)q;
  (void)t;
  (void)v;
  return AR_UNIMPLEMENTED;
}
int ar_cancel(struct ar_queue *q, struct ar_ticket t) {
  (void)q;
  (void)t;
  return AR_UNIMPLEMENTED;
}
int ar_pop(struct ar_queue *q, struct ap_value *v) {
  (void)q;
  (void)v;
  return AR_UNIMPLEMENTED;
}
