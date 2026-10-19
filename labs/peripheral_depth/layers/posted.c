#include "posted.h"
int pl_flush(PlPosted *a) {
  (void)a;
  return PL_UNIMPLEMENTED;
}
int pl_post_write(PlPosted *a, unsigned b, unsigned c, uint32_t d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return PL_UNIMPLEMENTED;
}
int pl_post_read(PlPosted *a, unsigned b, unsigned c, uint32_t *d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return PL_UNIMPLEMENTED;
}
int pl_exchange_begin(PlExchange *a, PlPosted *b, uint8_t c, unsigned d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return PL_UNIMPLEMENTED;
}
int pl_exchange_step(PlExchange *a, PlPosted *b, uint8_t *c) {
  (void)a;
  (void)b;
  (void)c;
  return PL_UNIMPLEMENTED;
}
