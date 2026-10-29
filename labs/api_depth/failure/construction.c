#include "construction.h"
void ae_construction_init(struct ae_construction *c, struct ac_ledger *l) {
  (void)c;
  (void)l;
}
enum ae_code ae_construct(struct ae_construction *c, unsigned n, unsigned s, unsigned f,
                    unsigned k, enum ae_construction_style t,
                    struct ae_construction_handle *o) {
  (void)c;
  (void)n;
  (void)s;
  (void)f;
  (void)k;
  (void)t;
  (void)o;
  return AE_UNIMPLEMENTED;
}
enum ae_code ae_cleanup(struct ae_construction *c, unsigned f) {
  (void)c;
  (void)f;
  return AE_UNIMPLEMENTED;
}
enum ae_code ae_construction_read(struct ae_construction_handle h, unsigned *o) {
  (void)h;
  (void)o;
  return AE_UNIMPLEMENTED;
}
enum ae_code ae_transfer(struct ae_construction *c, unsigned r) {
  (void)c;
  (void)r;
  return AE_UNIMPLEMENTED;
}
enum ae_code ae_after_publish(struct ae_construction *c, unsigned f) {
  (void)c;
  (void)f;
  return AE_UNIMPLEMENTED;
}
enum ae_code ae_compensate(struct ae_construction *c, unsigned p) {
  (void)c;
  (void)p;
  return AE_UNIMPLEMENTED;
}
