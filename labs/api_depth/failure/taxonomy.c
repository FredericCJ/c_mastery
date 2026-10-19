#include "taxonomy.h"
/* These are intentionally unfinished learner mechanisms. */
AeCode ae_map(unsigned b, int c) {
  (void)b;
  (void)c;
  return AE_UNIMPLEMENTED;
}
AeOutcome ae_outcome(AeCode c) {
  (void)c;
  return AE_TERMINAL;
}
AeCode ae_first(AeError *e, AeCode c, unsigned o, unsigned s, unsigned b,
                int x) {
  (void)e;
  (void)c;
  (void)o;
  (void)s;
  (void)b;
  (void)x;
  return AE_UNIMPLEMENTED;
}
AeCode ae_secondary(AeError *e, unsigned s, int c) {
  (void)e;
  (void)s;
  (void)c;
  return AE_UNIMPLEMENTED;
}
AeCode ae_boundary(const unsigned char *p, size_t n, unsigned c, unsigned a,
                   unsigned d, unsigned *o) {
  (void)p;
  (void)n;
  (void)c;
  (void)a;
  (void)d;
  (void)o;
  return AE_UNIMPLEMENTED;
}
AeCode ae_admit_digit(const unsigned char *p, size_t n, AeDigit *o) {
  (void)p;
  (void)n;
  (void)o;
  return AE_UNIMPLEMENTED;
}
AeCode ae_trusted_digit(AeDigit d, unsigned *o) {
  (void)d;
  (void)o;
  return AE_UNIMPLEMENTED;
}
AeCode ae_submit(AeCounter *c, unsigned i, unsigned d, AeTransportMode m) {
  (void)c;
  (void)i;
  (void)d;
  (void)m;
  return AE_UNIMPLEMENTED;
}
AeCode ae_write_result(size_t o, size_t a, AeCode c, AeWriteResult *r) {
  (void)o;
  (void)a;
  (void)c;
  (void)r;
  return AE_UNIMPLEMENTED;
}
AeLookup ae_lookup(unsigned k) {
  (void)k;
  return (AeLookup){0, 0};
}
AeCode ae_fields(const AvText *f, size_t n, AeFields *o) {
  (void)f;
  (void)n;
  (void)o;
  return AE_UNIMPLEMENTED;
}
