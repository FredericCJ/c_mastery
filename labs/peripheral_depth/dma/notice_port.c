#include "dma.h"
int pd_h745_notice(unsigned a, uint32_t b, uint32_t c, const struct pd_notice *d,
                   struct pd_notice *e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return PD_UNIMPLEMENTED;
}

int pd_h745_notice_cold_start(uint32_t epoch) {
  (void)epoch;
  return PD_UNIMPLEMENTED;
}
