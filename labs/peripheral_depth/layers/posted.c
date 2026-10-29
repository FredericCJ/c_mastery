#include "posted.h"
int pl_flush(struct pl_posted *a) {
  (void)a;
  return PL_UNIMPLEMENTED;
}
int pl_post_write(struct pl_posted *a, unsigned b, unsigned c, uint32_t d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return PL_UNIMPLEMENTED;
}
int pl_post_read(struct pl_posted *a, unsigned b, unsigned c, uint32_t *d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return PL_UNIMPLEMENTED;
}
int pl_exchange_begin(struct pl_exchange *a, struct pl_posted *b, uint8_t c, unsigned d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return PL_UNIMPLEMENTED;
}
int pl_exchange_step(struct pl_exchange *a, struct pl_posted *b, uint8_t *c) {
  (void)a;
  (void)b;
  (void)c;
  return PL_UNIMPLEMENTED;
}
