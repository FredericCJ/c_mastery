#include "target.h"
int pl_f439_open(PlUart *u, const PuClock *c, uint32_t b, unsigned k,
                 unsigned p) {
  (void)u;
  (void)c;
  (void)b;
  (void)k;
  (void)p;
  return PL_UNIMPLEMENTED;
}
PlTransport pl_f439_transport(PlUart *u) {
  (void)u;
  return (PlTransport){0};
}
