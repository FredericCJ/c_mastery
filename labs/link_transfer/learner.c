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
int lt_sizes(uint32_t n, const struct lt_layer *p, size_t c, uint32_t *o) {
  (void)n;
  (void)p;
  (void)c;
  (void)o;
  return LT_STUB;
}
int lt_begin(struct lt_borrow *s, const unsigned char *p, size_t n) {
  (void)s;
  (void)p;
  (void)n;
  return LT_STUB;
}
int lt_offer(struct lt_borrow *s, const unsigned char **p, size_t *n) {
  (void)s;
  (void)p;
  (void)n;
  return LT_STUB;
}
int lt_accept(struct lt_borrow *s, size_t n) {
  (void)s;
  (void)n;
  return LT_STUB;
}
int lt_cancel(struct lt_borrow *s, unsigned n) {
  (void)s;
  (void)n;
  return LT_STUB;
}
int lt_bridge(struct lt_bridge *s, unsigned v, const uint8_t *a, const uint8_t *b,
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
int lt_dp83848(const struct ed_bus *b, uint32_t h, unsigned p, unsigned n, uint32_t t,
               struct ed_phy *o) {
  (void)b;
  (void)h;
  (void)p;
  (void)n;
  (void)t;
  (void)o;
  return LT_STUB;
}
int lt_partial_begin(struct lt_partial *s, struct e7_sender *p, uint32_t n) {
  (void)s;
  (void)p;
  (void)n;
  return LT_STUB;
}
int lt_partial_progress(struct lt_partial *s, size_t p, uint32_t n) {
  (void)s;
  (void)p;
  (void)n;
  return LT_STUB;
}
int lt_partial_abort(struct lt_partial *s, unsigned p, uint32_t n) {
  (void)s;
  (void)p;
  (void)n;
  return LT_STUB;
}
