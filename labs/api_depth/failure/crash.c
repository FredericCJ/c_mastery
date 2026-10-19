#include "crash.h"
AeCode ae_fault_encode(AeFault f, unsigned char *p, size_t n) {
  (void)f;
  (void)p;
  (void)n;
  return AE_UNIMPLEMENTED;
}
AeCode ae_fault_decode(const unsigned char *p, size_t n, AeFault *f) {
  (void)p;
  (void)n;
  (void)f;
  return AE_UNIMPLEMENTED;
}
AeCode ae_fault_select(const AeCrashSlots *s, AeFault *f, unsigned *g) {
  (void)s;
  (void)f;
  (void)g;
  return AE_UNIMPLEMENTED;
}
AeCode ae_fault_save(AeCrashSlots *s, AeFault f, unsigned c) {
  (void)s;
  (void)f;
  (void)c;
  return AE_UNIMPLEMENTED;
}
void ae_recovery_init(AeRecovery *r) { (void)r; }
AeCode ae_recover(AeRecovery *r, const unsigned char *p, size_t n, unsigned b,
                  unsigned c) {
  (void)r;
  (void)p;
  (void)n;
  (void)b;
  (void)c;
  return AE_UNIMPLEMENTED;
}
