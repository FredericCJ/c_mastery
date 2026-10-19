#include "representation.h"
int rv_import(CvmValue v, RvValue *o) {
  (void)v;
  (void)o;
  return RV_UNIMPLEMENTED;
}
int rv_equal(RvValue a, RvValue b, int *o) {
  (void)a;
  (void)b;
  (void)o;
  return RV_UNIMPLEMENTED;
}
int rv_arithmetic(unsigned p, RvValue a, RvValue b, RvValue *o) {
  (void)p;
  (void)a;
  (void)b;
  (void)o;
  return RV_UNIMPLEMENTED;
}
int rv_decode(const uint8_t *b, size_t n, RvImage *o) {
  (void)b;
  (void)n;
  (void)o;
  return RV_UNIMPLEMENTED;
}
int rv_encode(const RvImage *i, uint8_t *b, size_t n, size_t *o) {
  (void)i;
  (void)b;
  (void)n;
  (void)o;
  return RV_UNIMPLEMENTED;
}
