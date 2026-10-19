#include "driver.h"
/* Intentionally unfinished. Implement the composed ownership protocol. */
int nb_init(NbNet *s, EnRing *r, NaConfig c, uint32_t e, uint32_t i, unsigned n,
            uint64_t t) {
  (void)s;
  (void)r;
  (void)c;
  (void)e;
  (void)i;
  (void)n;
  (void)t;
  return NB_STUB;
}
int nb_step(NbNet *s, uint64_t t) {
  (void)s;
  (void)t;
  return NB_STUB;
}
int nb_offer(NbNet *s, const void *p, size_t n) {
  (void)s;
  (void)p;
  (void)n;
  return NB_STUB;
}
int nb_http_pump(NbNet *s, HttpParser *p, size_t b, size_t *n, int *e) {
  (void)s;
  (void)p;
  (void)b;
  (void)n;
  (void)e;
  return NB_STUB;
}
int nb_close(NbNet *s) {
  (void)s;
  return NB_STUB;
}
int nb_stop(NbNet *s, uint64_t t) {
  (void)s;
  (void)t;
  return NB_STUB;
}
int nb_stop_step(NbNet *s, uint64_t t) {
  (void)s;
  (void)t;
  return NB_STUB;
}
int nb_restart(NbNet *s, uint32_t e, uint32_t i, unsigned n, uint64_t t) {
  (void)s;
  (void)e;
  (void)i;
  (void)n;
  (void)t;
  return NB_STUB;
}
