#include "congestion.h"
int nr_init(NrControl *s, uint32_t m, uint32_t i, uint32_t w) {
  (void)s;
  (void)m;
  (void)i;
  (void)w;
  return NR_STUB;
}
int nr_policy(NrControl *s, unsigned e) {
  (void)s;
  (void)e;
  return NR_STUB;
}
uint32_t nr_flight(const NrControl *s) {
  (void)s;
  return 0;
}
uint32_t nr_usable(const NrControl *s) {
  (void)s;
  return 0;
}
int nr_send(NrControl *s, uint32_t n, unsigned c, uint64_t t, uint32_t *q) {
  (void)s;
  (void)n;
  (void)c;
  (void)t;
  (void)q;
  return NR_STUB;
}
int nr_ack(NrControl *s, uint32_t a, uint32_t w, unsigned p, unsigned f,
           uint64_t t) {
  (void)s;
  (void)a;
  (void)w;
  (void)p;
  (void)f;
  (void)t;
  return NR_STUB;
}
int nr_timeout(NrControl *s, uint64_t t) {
  (void)s;
  (void)t;
  return NR_STUB;
}
int nr_restart(NrControl *s, uint64_t t, uint32_t r) {
  (void)s;
  (void)t;
  (void)r;
  return NR_STUB;
}
