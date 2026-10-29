#ifndef PA_ADC_H
#define PA_ADC_H
#include "uart.h"
#include <stdint.h>
enum {
  PA_OK,
  PA_INVALID,
  PA_BUSY,
  PA_STALE,
  PA_TIMEOUT,
  PA_OVERRUN,
  PA_UNIMPLEMENTED
};
enum {
  PA_SR,
  PA_CR1,
  PA_CR2,
  PA_SMPR1,
  PA_SMPR2,
  PA_SQR1,
  PA_SQR2,
  PA_SQR3,
  PA_DR,
  PA_CCR
};
enum { PA_EOC = 2, PA_OVR = 32, PA_ADON = 1, PA_EOCS = 1024, PA_SCAN = 256 };
#define PA_SWSTART (UINT32_C(1) << 30)
#define PA_EXTERNAL (UINT32_C(0x18) << 24)
struct pa_io {
  void *context;
  uint32_t (*read)(void *, unsigned);
  void (*write)(void *, unsigned, uint32_t);
};
struct pa_config {
  unsigned sample, channels, timer;
};
struct pa_timing {
  uint32_t hz, total_cycles, conversion_us;
  unsigned prescaler;
};
struct pa_adc {
  uint32_t generation, last_token, token, last, due, deadline;
  unsigned phase, received, channels, timer, warm_sampled;
  uint16_t partial[2], *output;
};
int pa_timing(const struct pu_clock *, const struct pa_config *, struct pa_timing *);
int pa_open(struct pa_adc *, struct pa_io, const struct pu_clock *, const struct pa_config *, uint32_t now_us);
int pa_ready(struct pa_adc *, uint32_t now_us);
int pa_begin(struct pa_adc *, struct pa_io, uint32_t generation, uint32_t token, uint16_t *,
             uint32_t now_us, uint32_t budget_us);
int pa_step(struct pa_adc *, struct pa_io, uint32_t token, uint32_t now_us);
int pa_cancel(struct pa_adc *, struct pa_io, uint32_t token);
/* A two-point engineering correction, not a self-calibration peripheral. */
int pa_calibrate(unsigned code, unsigned low_code, unsigned high_code,
                 unsigned low_mv, unsigned high_mv, int32_t *mv);
int pa_ramp(unsigned index, unsigned steps, unsigned low, unsigned high,
            unsigned *code);
/* Synthetic Schmitt model: inclusive rising/lowering thresholds, state retained
   between them; no claim that a comparator's typ. hysteresis is guaranteed. */
int pa_threshold(unsigned *state, int32_t input, int32_t low, int32_t high);
struct pa_io pa_f439_io(void);
void pa_f439_pins(void);
int pa_f439_timer(unsigned prescaler, unsigned reload, unsigned enabled);
void pa_f439_dac_open(void);
int pa_f439_dac_write(unsigned code);
unsigned pa_f439_dac_read(void);
struct pa_comparator {
  uint32_t last, due;
  unsigned phase, warm_sampled;
};
int pa_g474_comp_open(struct pa_comparator *, unsigned hysteresis_code);
int pa_g474_comp_poll(struct pa_comparator *, uint32_t now_us, unsigned *level);
int pa_g474_comp_close(struct pa_comparator *);
#endif
