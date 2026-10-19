#include "target.h"
#ifndef PL_TARGET_MUTANT
#define PL_TARGET_MUTANT 0
#endif
int pl_f439_open(PlUart *u, const PuClock *c, uint32_t b, unsigned k,
                 unsigned p) {
  /* GPIO/pin and clock ownership established by the board contract before open.
   */
  return pl_uart_open(u, pu_f439_io(), c, b, k, p);
}
static int submit(void *ctx, const uint8_t *bytes, unsigned n, uint32_t token,
                  uint32_t now, uint32_t budget) {
  PlUart *u = ctx;
  uintptr_t address = (uintptr_t)bytes;
  if (u->backend == PU_DMA && PL_TARGET_MUTANT != 1 &&
      (!n || n > 8 || address < 0x20000000u || address >= 0x20030000u ||
       n > 0x20030000u - address))
    return PL_INVALID;
  PlTransport t = pl_uart_transport(u);
  return t.submit(t.context, bytes, n, token, now, budget);
}
PlTransport pl_f439_transport(PlUart *u) {
  PlTransport t = pl_uart_transport(u);
  if (u)
    t.submit = submit;
  return t;
}
