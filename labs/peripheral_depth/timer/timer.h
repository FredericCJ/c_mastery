#ifndef PERIPHERAL_TIMER_DEPTH_H
#define PERIPHERAL_TIMER_DEPTH_H
#include "uart.h"
#include <stdint.h>
enum {
  PT_OK,
  PT_INVALID,
  PT_RANGE,
  PT_STALE,
  PT_BUSY,
  PT_LOST,
  PT_UNIMPLEMENTED
};
typedef struct {
  uint32_t clock, generation;
  unsigned psc, arr, ccr[2], center;
} PtConfig;
typedef struct {
  uint64_t numerator, denominator;
} PtRatio;
int pt_choose(const PuClock *, unsigned psc, unsigned hz, unsigned duty0,
              unsigned duty1, unsigned center, unsigned max_ppm, PtConfig *);
int pt_ratios(const PtConfig *, PtRatio *frequency, PtRatio *tick,
              PtRatio duty[2]);
typedef struct {
  PtConfig active, shadow;
  unsigned counter, down, prescale, udis, urs, uif, running;
  uint32_t updates;
} PtModel;
int pt_init(PtModel *, const PtConfig *);
int pt_stage(PtModel *, const PtConfig *);
int pt_tick(PtModel *);  /* one counter tick, not one input clock */
int pt_input(PtModel *); /* one timer input clock */
int pt_ug(PtModel *);
int pt_level(const PtModel *, unsigned channel, unsigned *);
int pt_interval(unsigned previous, unsigned current, unsigned max_gap,
                unsigned overcapture, unsigned *ticks);
/* All methods serialized by the caller, including clock generation changes. */
int pt_f439_start(const PtConfig *);
int pt_f439_stage(const PtConfig *, uint32_t current_generation);
void pt_f439_stop(void);
int pt_f439_capture_start(unsigned psc);
int pt_f439_capture(unsigned *value);
void pt_f439_pins(void);
#endif
