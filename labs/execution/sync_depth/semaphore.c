#include "semaphore.h"
int cds_init(struct cd_sem *s, cmx_kernel *k, unsigned initial, unsigned maximum,
             unsigned priority) {
  (void)s;
  (void)k;
  (void)initial;
  (void)maximum;
  (void)priority;
  return CMX_UNIMPLEMENTED;
}
int cds_take(struct cd_sem *s, unsigned t, cmx_deadline d, struct cw_token *out) {
  (void)s;
  (void)t;
  (void)d;
  (void)out;
  return CMX_UNIMPLEMENTED;
}
int cds_post(struct cd_sem *s, struct cd_post *out) {
  (void)s;
  (void)out;
  return CMX_UNIMPLEMENTED;
}
int cds_close(struct cd_sem *s) {
  (void)s;
  return CMX_UNIMPLEMENTED;
}
int cds_deferred_init(struct cd_deferred *d, unsigned maximum) {
  (void)d;
  (void)maximum;
  return CMX_UNIMPLEMENTED;
}
int cds_deferred_offer(struct cd_deferred *d) {
  (void)d;
  return CMX_UNIMPLEMENTED;
}
int cds_deferred_drain(struct cd_deferred *d, struct cd_sem *s, unsigned budget,
                       unsigned *out) {
  (void)d;
  (void)s;
  (void)budget;
  (void)out;
  return CMX_UNIMPLEMENTED;
}
int cds_deferred_close(struct cd_deferred *d) {
  (void)d;
  return CMX_UNIMPLEMENTED;
}
