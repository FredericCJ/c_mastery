#include "crash.h"
enum ae_code ae_fault_encode(struct ae_fault f, unsigned char *p, size_t n) {
  (void)f;
  (void)p;
  (void)n;
  return AE_UNIMPLEMENTED;
}
enum ae_code ae_fault_decode(const unsigned char *p, size_t n, struct ae_fault *f) {
  (void)p;
  (void)n;
  (void)f;
  return AE_UNIMPLEMENTED;
}
enum ae_code ae_fault_select(const struct ae_crash_slots *s, struct ae_fault *f, unsigned *g) {
  (void)s;
  (void)f;
  (void)g;
  return AE_UNIMPLEMENTED;
}
enum ae_code ae_fault_save(struct ae_crash_slots *s, struct ae_fault f, unsigned c) {
  (void)s;
  (void)f;
  (void)c;
  return AE_UNIMPLEMENTED;
}
void ae_recovery_init(struct ae_recovery *r) { (void)r; }
enum ae_code ae_recover(struct ae_recovery *r, const unsigned char *p, size_t n, unsigned b,
                  unsigned c) {
  (void)r;
  (void)p;
  (void)n;
  (void)b;
  (void)c;
  return AE_UNIMPLEMENTED;
}
