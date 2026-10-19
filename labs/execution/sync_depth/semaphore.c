#include "semaphore.h"
int cds_init(CDSem *s, cmx_kernel *k, unsigned initial, unsigned maximum,
             unsigned priority) {
  (void)s;
  (void)k;
  (void)initial;
  (void)maximum;
  (void)priority;
  return CMX_UNIMPLEMENTED;
}
int cds_take(CDSem *s, unsigned t, cmx_deadline d, CWToken *out) {
  (void)s;
  (void)t;
  (void)d;
  (void)out;
  return CMX_UNIMPLEMENTED;
}
int cds_post(CDSem *s, CDPost *out) {
  (void)s;
  (void)out;
  return CMX_UNIMPLEMENTED;
}
int cds_close(CDSem *s) {
  (void)s;
  return CMX_UNIMPLEMENTED;
}
int cds_deferred_init(CDDeferred *d, unsigned maximum) {
  (void)d;
  (void)maximum;
  return CMX_UNIMPLEMENTED;
}
int cds_deferred_offer(CDDeferred *d) {
  (void)d;
  return CMX_UNIMPLEMENTED;
}
int cds_deferred_drain(CDDeferred *d, CDSem *s, unsigned budget,
                       unsigned *out) {
  (void)d;
  (void)s;
  (void)budget;
  (void)out;
  return CMX_UNIMPLEMENTED;
}
int cds_deferred_close(CDDeferred *d) {
  (void)d;
  return CMX_UNIMPLEMENTED;
}
