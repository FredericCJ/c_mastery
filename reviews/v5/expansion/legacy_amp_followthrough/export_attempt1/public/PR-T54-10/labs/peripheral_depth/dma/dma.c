#include "dma.h"
int pd_validate(unsigned a, unsigned b, const PdExtent *c, unsigned d,
                uint32_t *e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return PD_UNIMPLEMENTED;
}
int pd_submit(PdTransfer *a, PdIo b, unsigned c, unsigned d, const PdExtent *e,
              unsigned f, uint32_t g) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  (void)f;
  (void)g;
  return PD_UNIMPLEMENTED;
}
int pd_poll(PdTransfer *a, PdIo b, uint32_t c) {
  (void)a;
  (void)b;
  (void)c;
  return PD_UNIMPLEMENTED;
}
int pd_cancel(PdTransfer *a, PdIo b, uint32_t c) {
  (void)a;
  (void)b;
  (void)c;
  return PD_UNIMPLEMENTED;
}
int pd_finalize(PdTransfer *a, PdIo b, uint32_t c, PdResult *d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return PD_UNIMPLEMENTED;
}
int pd_suffix(const PdExtent *a, unsigned b, uint32_t c, PdExtent *d,
              unsigned *e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return PD_UNIMPLEMENTED;
}
