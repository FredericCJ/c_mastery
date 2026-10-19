#include "observability.h"
AeCode ae_record_encode(const AeRecord *r, unsigned char *p, size_t n) {
  (void)r;
  (void)p;
  (void)n;
  return AE_UNIMPLEMENTED;
}
AeCode ae_record_decode(const unsigned char *p, size_t n, AeRecord *r) {
  (void)p;
  (void)n;
  (void)r;
  return AE_UNIMPLEMENTED;
}
AeCode ae_tick_order(unsigned a, unsigned b, unsigned w, int *o) {
  (void)a;
  (void)b;
  (void)w;
  (void)o;
  return AE_UNIMPLEMENTED;
}
AeCode ae_rate_refill(AeRate *r, unsigned n) {
  (void)r;
  (void)n;
  return AE_UNIMPLEMENTED;
}
AeCode ae_log(AeLog *l, const AeRecord *r, AeLogPolicy p) {
  (void)l;
  (void)r;
  (void)p;
  return AE_UNIMPLEMENTED;
}
AeCode ae_log_drain(AeLog *l, AeSink s, void *c) {
  (void)l;
  (void)s;
  (void)c;
  return AE_UNIMPLEMENTED;
}
AeCode ae_report(AeReporter *r, const AeRecord *x, AeSink s, void *c) {
  (void)r;
  (void)x;
  (void)s;
  (void)c;
  return AE_UNIMPLEMENTED;
}
