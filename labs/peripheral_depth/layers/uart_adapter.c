#include "uart_adapter.h"
int pl_uart_open(PlUart *a, PuIo b, const PuClock *c, uint32_t d, unsigned e,
                 unsigned f) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  (void)f;
  return PL_UNIMPLEMENTED;
}
PlTransport pl_uart_transport(PlUart *u) {
  (void)u;
  PlTransport t = {0};
  return t;
}
