#include "cvm.h"
/* Learner implementation. Stubs compile; contract tests must fail until
 * implemented. */
CvmValue cvm_int(int32_t n) {
  CvmValue v = {CVM_INT, n, 0};
  return v;
}
void cvm_init(Cvm *v, uint32_t f, uint32_t g, uint32_t h) {
  (void)v;
  (void)f;
  (void)g;
  (void)h;
}
CvmStatus cvm_decode(const uint8_t *b, size_t n, CvmProgram *p) {
  (void)b;
  (void)n;
  (void)p;
  return CVM_NOT_IMPLEMENTED;
}
CvmStatus cvm_encode(const CvmProgram *p, uint8_t *b, size_t n, size_t *w) {
  (void)p;
  (void)b;
  (void)n;
  (void)w;
  return CVM_NOT_IMPLEMENTED;
}
CvmStatus cvm_load(const uint8_t *b, size_t n, CvmProgram *p) {
  (void)b;
  (void)n;
  (void)p;
  return CVM_NOT_IMPLEMENTED;
}
CvmStatus cvm_save(const CvmProgram *p, uint8_t *b, size_t n, size_t *w) {
  (void)p;
  (void)b;
  (void)n;
  (void)w;
  return CVM_NOT_IMPLEMENTED;
}
CvmStatus cvm_verify(const CvmProgram *p) {
  (void)p;
  return CVM_NOT_IMPLEMENTED;
}
CvmStatus cvm_run(Cvm *v, const CvmProgram *p, CvmValue *r) {
  (void)v;
  (void)p;
  (void)r;
  return CVM_NOT_IMPLEMENTED;
}
CvmStatus cvm_collect(Cvm *v) {
  (void)v;
  return CVM_NOT_IMPLEMENTED;
}
CvmStatus cvm_pair(Cvm *v, CvmValue a, CvmValue b, CvmValue *r) {
  (void)v;
  (void)a;
  (void)b;
  (void)r;
  return CVM_NOT_IMPLEMENTED;
}
CvmStatus cvm_root_push(Cvm *v, CvmValue a) {
  (void)v;
  (void)a;
  return CVM_NOT_IMPLEMENTED;
}
void cvm_root_pop(Cvm *v) { (void)v; }
CvmStatus cvm_field(Cvm *v, CvmValue r, int right, CvmValue *out) {
  (void)v;
  (void)r;
  (void)right;
  (void)out;
  return CVM_NOT_IMPLEMENTED;
}
const char *cvm_status_name(CvmStatus s) {
  static const char *const names[] = {
      "OK",          "FORMAT",      "VERIFY", "TYPE",       "RANGE",
      "STACK_LIMIT", "FRAME_LIMIT", "FUEL",   "HEAP_LIMIT", "GC_BUDGET",
      "HOST_BUDGET", "HOST_ERROR",  "STALE",  "SYNTAX",     "NOT_IMPLEMENTED"};
  return (unsigned)s < sizeof names / sizeof names[0] ? names[s] : "UNKNOWN";
}
