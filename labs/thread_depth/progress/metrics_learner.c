#include "metrics.h"
int tp_separated(uintptr_t a, size_t b, size_t c, size_t d, size_t e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return -1;
}
uint32_t tp_mix(uint32_t a, unsigned b) {
  (void)a;
  (void)b;
  return 0;
}
int tp_accumulate(const unsigned *a, size_t n, unsigned b, unsigned c,
                  unsigned d, unsigned e, unsigned f, _Atomic(uint64_t) *g,
                  TPWork *h) {
  (void)a;
  (void)n;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  (void)f;
  (void)g;
  (void)h;
  return TP_INVALID;
}
int tp_total_init(TPTotal *p) {
  (void)p;
  return TP_INVALID;
}
int tp_total_add(TPTotal *p, uint64_t n) {
  (void)p;
  (void)n;
  return TP_INVALID;
}
int tp_total_read(TPTotal *p, uint64_t *a, uint64_t *b) {
  (void)p;
  (void)a;
  (void)b;
  return TP_INVALID;
}
int tp_total_destroy(TPTotal *p) {
  (void)p;
  return TP_INVALID;
}
