#include "ring.h"
int ar_ring_init(ArRingPolicy *r, size_t n) {
  (void)r;
  (void)n;
  return AR_UNIMPLEMENTED;
}
int ar_ring_push(ArRingPolicy *r, ArItem i, unsigned policy, ArEviction *out) {
  (void)r;
  (void)i;
  (void)policy;
  (void)out;
  return AR_UNIMPLEMENTED;
}
int ar_ring_pop(ArRingPolicy *r, ArItem *out) {
  (void)r;
  (void)out;
  return AR_UNIMPLEMENTED;
}
