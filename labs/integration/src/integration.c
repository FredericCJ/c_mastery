#include "integration.h"
int in_records(const uint8_t *p, size_t n, algo_ring *q, size_t *u) {
  (void)p;
  (void)n;
  (void)q;
  (void)u;
  return -1;
}
int in_drain(algo_ring *q, InTransfer *t, FWriter w, void *p, unsigned n) {
  (void)q;
  (void)t;
  (void)w;
  (void)p;
  (void)n;
  return -1;
}
int in_stream(InStream *s, uint8_t b, algo_ring *q) {
  (void)s;
  (void)b;
  (void)q;
  return -1;
}
int in_shutdown(algo_ring *q, FAsync *s, unsigned c, uint32_t t, unsigned *d) {
  (void)q;
  (void)s;
  (void)c;
  (void)t;
  (void)d;
  return -1;
}
int in_filter(V4Filter *f, const int32_t *i, size_t n, int32_t *o, size_t c,
              const V4Sample *a, const V4Sample *b, V4Measurement *m) {
  (void)f;
  (void)i;
  (void)n;
  (void)o;
  (void)c;
  (void)a;
  (void)b;
  (void)m;
  return -1;
}
#ifdef IN_FIRMWARE
int in_dma_handoff(FwDma *d, FwIo i, uint32_t t, algo_ring *q) {
  (void)d;
  (void)i;
  (void)t;
  (void)q;
  return -1;
}
int in_gateway(FwIo i, algo_ring *q, V4Filter *f, FwWatchdog *w, int32_t *o) {
  (void)i;
  (void)q;
  (void)f;
  (void)w;
  (void)o;
  return -1;
}
#endif
#ifdef IN_VM_STORE
int in_vm_store(const char *s, size_t n, uint8_t *p, size_t c) {
  (void)s;
  (void)n;
  (void)p;
  (void)c;
  return -1;
}
#endif
