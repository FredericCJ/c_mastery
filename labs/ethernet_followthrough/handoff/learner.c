#include "handoff.h"
int eh_next(const EhRoute *a, uint32_t b, uint32_t *c) {
  (void)a;
  (void)b;
  (void)c;
  return EH_STUB;
}
int eh_frame(const EhRoute *a, const unsigned char *b, const EhNeighbor *c,
             const unsigned char *d, size_t e, unsigned char *f, size_t g,
             size_t *h) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  (void)f;
  (void)g;
  (void)h;
  return EH_STUB;
}
int eh_correlate(const EhClock *a, const uint32_t *b, size_t c, uint64_t *d,
                 uint64_t *e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return EH_STUB;
}
