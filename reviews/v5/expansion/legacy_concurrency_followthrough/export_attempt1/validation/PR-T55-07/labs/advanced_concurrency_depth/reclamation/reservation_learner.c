#include "reservation.h"
void ar_init(ARQueue *q, APHook h, void *c) {
  (void)q;
  (void)h;
  (void)c;
}
int ar_reserve(ARQueue *q, ARTicket *t) {
  (void)q;
  (void)t;
  return AR_UNIMPLEMENTED;
}
int ar_finish(ARQueue *q, ARTicket t, APValue v) {
  (void)q;
  (void)t;
  (void)v;
  return AR_UNIMPLEMENTED;
}
int ar_cancel(ARQueue *q, ARTicket t) {
  (void)q;
  (void)t;
  return AR_UNIMPLEMENTED;
}
int ar_pop(ARQueue *q, APValue *v) {
  (void)q;
  (void)v;
  return AR_UNIMPLEMENTED;
}
