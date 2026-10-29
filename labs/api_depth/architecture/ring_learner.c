#include "ring.h"
int ar_ring_init(struct ar_ring_policy *r, size_t n) {
  (void)r;
  (void)n;
  return AR_UNIMPLEMENTED;
}
int ar_ring_push(struct ar_ring_policy *r, struct ar_item i, unsigned policy, struct ar_eviction *out) {
  (void)r;
  (void)i;
  (void)policy;
  (void)out;
  return AR_UNIMPLEMENTED;
}
int ar_ring_pop(struct ar_ring_policy *r, struct ar_item *out) {
  (void)r;
  (void)out;
  return AR_UNIMPLEMENTED;
}
