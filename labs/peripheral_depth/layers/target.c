#include "target.h"
int pl_f439_open(struct pl_uart *u, const struct pu_clock *c, uint32_t b, unsigned k,
                 unsigned p) {
  (void)u;
  (void)c;
  (void)b;
  (void)k;
  (void)p;
  return PL_UNIMPLEMENTED;
}
struct pl_transport pl_f439_transport(struct pl_uart *u) {
  (void)u;
  return (struct pl_transport){0};
}
