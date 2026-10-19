#include "transfer.h"
int lt_order(unsigned m, unsigned a, unsigned b, int *o) {
  (void)m;
  (void)a;
  (void)b;
  (void)o;
  return LT_STUB;
}
int lt_ack(unsigned m, unsigned *a, unsigned *b, unsigned n) {
  (void)m;
  (void)a;
  (void)b;
  (void)n;
  return LT_STUB;
}
int lt_sizes(uint32_t n, const LtLayer *p, size_t c, uint32_t *o) {
  (void)n;
  (void)p;
  (void)c;
  (void)o;
  return LT_STUB;
}
int lt_begin(LtBorrow *s, const unsigned char *p, size_t n) {
  (void)s;
  (void)p;
  (void)n;
  return LT_STUB;
}
int lt_offer(LtBorrow *s, const unsigned char **p, size_t *n) {
  (void)s;
  (void)p;
  (void)n;
  return LT_STUB;
}
int lt_accept(LtBorrow *s, size_t n) {
  (void)s;
  (void)n;
  return LT_STUB;
}
int lt_cancel(LtBorrow *s, unsigned n) {
  (void)s;
  (void)n;
  return LT_STUB;
}
int lt_bridge(LtBridge *s, unsigned v, const uint8_t *a, const uint8_t *b,
              unsigned p, uint32_t n, unsigned *m) {
  (void)s;
  (void)v;
  (void)a;
  (void)b;
  (void)p;
  (void)n;
  (void)m;
  return LT_STUB;
}
int lt_dp83848(const EDBus *b, uint32_t h, unsigned p, unsigned n, uint32_t t,
               EDPhy *o) {
  (void)b;
  (void)h;
  (void)p;
  (void)n;
  (void)t;
  (void)o;
  return LT_STUB;
}
int lt_partial_begin(LtPartial *s, E7Sender *p, uint32_t n) {
  (void)s;
  (void)p;
  (void)n;
  return LT_STUB;
}
int lt_partial_progress(LtPartial *s, size_t p, uint32_t n) {
  (void)s;
  (void)p;
  (void)n;
  return LT_STUB;
}
int lt_partial_abort(LtPartial *s, unsigned p, uint32_t n) {
  (void)s;
  (void)p;
  (void)n;
  return LT_STUB;
}
