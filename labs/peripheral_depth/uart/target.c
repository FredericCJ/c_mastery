#include "uart.h"
/* Intentionally unfinished learner adapter: implement inspected F439 accesses.
 */
PuIo pu_f439_io(void) {
  PuIo io = {0, 0, 0};
  return io;
}
void pu_f439_pins(void) {}
void pu_f439_rts_init(void) {}
int pu_f439_rts(const PuRx *q) {
  (void)q;
  return PU_UNIMPLEMENTED;
}
int pu_f439_dma_submit(PuDriver *d, uint32_t g, uint32_t t, const uint8_t *p,
                       unsigned n, uint32_t now, uint32_t budget) {
  (void)d;
  (void)g;
  (void)t;
  (void)p;
  (void)n;
  (void)now;
  (void)budget;
  return PU_UNIMPLEMENTED;
}
