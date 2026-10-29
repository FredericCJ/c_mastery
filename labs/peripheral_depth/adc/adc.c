#include "adc.h"
int pa_timing(const struct pu_clock *a, const struct pa_config *b, struct pa_timing *c) {
  (void)a;
  (void)b;
  (void)c;
  return PA_UNIMPLEMENTED;
}
int pa_open(struct pa_adc *a, struct pa_io b, const struct pu_clock *c, const struct pa_config *d, uint32_t e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return PA_UNIMPLEMENTED;
}
int pa_ready(struct pa_adc *a, uint32_t b) {
  (void)a;
  (void)b;
  return PA_UNIMPLEMENTED;
}
int pa_begin(struct pa_adc *a, struct pa_io b, uint32_t c, uint32_t d, uint16_t *e, uint32_t f,
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
int pa_step(struct pa_adc *a, struct pa_io b, uint32_t c, uint32_t d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return PA_UNIMPLEMENTED;
}
int pa_cancel(struct pa_adc *a, struct pa_io b, uint32_t c) {
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
