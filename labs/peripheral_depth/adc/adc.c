#include "adc.h"
int pa_timing(const PuClock *a, const PaConfig *b, PaTiming *c) {
  (void)a;
  (void)b;
  (void)c;
  return PA_UNIMPLEMENTED;
}
int pa_open(PaAdc *a, PaIo b, const PuClock *c, const PaConfig *d, uint32_t e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return PA_UNIMPLEMENTED;
}
int pa_ready(PaAdc *a, uint32_t b) {
  (void)a;
  (void)b;
  return PA_UNIMPLEMENTED;
}
int pa_begin(PaAdc *a, PaIo b, uint32_t c, uint32_t d, uint16_t *e, uint32_t f,
             uint32_t g) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  (void)f;
  (void)g;
  return PA_UNIMPLEMENTED;
}
int pa_step(PaAdc *a, PaIo b, uint32_t c, uint32_t d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return PA_UNIMPLEMENTED;
}
int pa_cancel(PaAdc *a, PaIo b, uint32_t c) {
  (void)a;
  (void)b;
  (void)c;
  return PA_UNIMPLEMENTED;
}
int pa_calibrate(unsigned a, unsigned b, unsigned c, unsigned d, unsigned e,
                 int32_t *f) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  (void)f;
  return PA_UNIMPLEMENTED;
}
int pa_ramp(unsigned a, unsigned b, unsigned c, unsigned d, unsigned *e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return PA_UNIMPLEMENTED;
}
int pa_threshold(unsigned *a, int32_t b, int32_t c, int32_t d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return PA_UNIMPLEMENTED;
}
