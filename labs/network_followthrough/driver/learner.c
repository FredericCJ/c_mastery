#include "driver.h"
/* Intentionally unfinished. Implement the composed ownership protocol. */
int nb_init(struct nb_net *s, struct en_ring *r, struct na_config c, uint32_t e, uint32_t i, unsigned n,
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
int nb_step(struct nb_net *s, uint64_t t) {
  (void)s;
  (void)t;
  return NB_STUB;
}
int nb_offer(struct nb_net *s, const void *p, size_t n) {
  (void)s;
  (void)p;
  (void)n;
  return NB_STUB;
}
int nb_http_pump(struct nb_net *s, struct http_parser *p, size_t b, size_t *n, int *e) {
  (void)s;
  (void)p;
  (void)b;
  (void)n;
  (void)e;
  return NB_STUB;
}
int nb_close(struct nb_net *s) {
  (void)s;
  return NB_STUB;
}
int nb_stop(struct nb_net *s, uint64_t t) {
  (void)s;
  (void)t;
  return NB_STUB;
}
int nb_stop_step(struct nb_net *s, uint64_t t) {
  (void)s;
  (void)t;
  return NB_STUB;
}
int nb_restart(struct nb_net *s, uint32_t e, uint32_t i, unsigned n, uint64_t t) {
  (void)s;
  (void)e;
  (void)i;
  (void)n;
  (void)t;
  return NB_STUB;
}
