#include "object_runtime_generational.h"
ORStatus or_nursery_cards_init(const ORNursery *h, ORCards *out) {
  (void)h;
  (void)out;
  return OR_STATE;
}
ORStatus or_nursery_old_store(ORNursery *h, ORCards *c, unsigned o, unsigned f,
                              unsigned t) {
  (void)h;
  (void)c;
  (void)o;
  (void)f;
  (void)t;
  return OR_STATE;
}
ORStatus or_minor_preflight(ORNursery *h, ORCards *c, unsigned y, unsigned o,
                            unsigned *n, unsigned *s) {
  (void)h;
  (void)c;
  (void)y;
  (void)o;
  (void)n;
  (void)s;
  return OR_STATE;
}
ORStatus or_major_preflight(ORNursery *h, ORCards *c, unsigned *n,
                            uint8_t f[16]) {
  (void)h;
  (void)c;
  (void)n;
  (void)f;
  return OR_STATE;
}
void or_large_init(ORLargeHeap *h) { (void)h; }
ORStatus or_large_allocate(ORLargeHeap *h, unsigned n, uint32_t p,
                           ORLargeHandle *o) {
  (void)h;
  (void)n;
  (void)p;
  (void)o;
  return OR_STATE;
}
ORStatus or_large_release(ORLargeHeap *h, ORLargeHandle p) {
  (void)h;
  (void)p;
  return OR_STATE;
}
ORStatus or_large_pin(ORLargeHeap *h, ORLargeHandle p, ORLargeLease *o,
                      const uint32_t **d, unsigned *n) {
  (void)h;
  (void)p;
  (void)o;
  (void)d;
  (void)n;
  return OR_STATE;
}
ORStatus or_large_unpin(ORLargeLease p) {
  (void)p;
  return OR_STATE;
}
ORStatus or_large_collect(ORLargeHeap *h, unsigned *n) {
  (void)h;
  (void)n;
  return OR_STATE;
}
