#include "construction.h"
void ae_construction_init(AeConstruction *c, AcLedger *l) {
  (void)c;
  (void)l;
}
AeCode ae_construct(AeConstruction *c, unsigned n, unsigned s, unsigned f,
                    unsigned k, AeConstructionStyle t,
                    AeConstructionHandle *o) {
  (void)c;
  (void)n;
  (void)s;
  (void)f;
  (void)k;
  (void)t;
  (void)o;
  return AE_UNIMPLEMENTED;
}
AeCode ae_cleanup(AeConstruction *c, unsigned f) {
  (void)c;
  (void)f;
  return AE_UNIMPLEMENTED;
}
AeCode ae_construction_read(AeConstructionHandle h, unsigned *o) {
  (void)h;
  (void)o;
  return AE_UNIMPLEMENTED;
}
AeCode ae_transfer(AeConstruction *c, unsigned r) {
  (void)c;
  (void)r;
  return AE_UNIMPLEMENTED;
}
AeCode ae_after_publish(AeConstruction *c, unsigned f) {
  (void)c;
  (void)f;
  return AE_UNIMPLEMENTED;
}
AeCode ae_compensate(AeConstruction *c, unsigned p) {
  (void)c;
  (void)p;
  return AE_UNIMPLEMENTED;
}
