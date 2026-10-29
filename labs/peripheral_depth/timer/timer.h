#ifndef PERIPHERAL_TIMER_DEPTH_H
#define PERIPHERAL_TIMER_DEPTH_H
#include "uart.h"
#include <stdint.h>

enum
{
    PT_OK,
    PT_INVALID,
    PT_RANGE,
    PT_STALE,
    PT_BUSY,
    PT_LOST,
    PT_UNIMPLEMENTED
};

struct pt_config
{
    uint32_t clock, generation;
    unsigned psc, arr, ccr[2], center;
};

struct pt_ratio
{
    uint64_t numerator, denominator;
};

int pt_choose(const struct pu_clock *, unsigned psc, unsigned hz,
              unsigned duty0, unsigned duty1, unsigned center, unsigned max_ppm,
              struct pt_config *);
int pt_ratios(const struct pt_config *, struct pt_ratio *frequency,
              struct pt_ratio *tick, struct pt_ratio duty[2]);

struct pt_model
{
    struct pt_config active, shadow;
    unsigned counter, down, prescale, udis, urs, uif, running;
    uint32_t updates;
};

int pt_init(struct pt_model *, const struct pt_config *);
int pt_stage(struct pt_model *, const struct pt_config *);
int pt_tick(struct pt_model *); /* one counter tick, not one input clock */
int pt_input(struct pt_model *); /* one timer input clock */
int pt_ug(struct pt_model *);
int pt_level(const struct pt_model *, unsigned channel, unsigned *);
int pt_interval(unsigned previous, unsigned current, unsigned max_gap,
                unsigned overcapture, unsigned *ticks);
/* All methods serialized by the caller, including clock generation changes. */
int pt_f439_start(const struct pt_config *);
int pt_f439_stage(const struct pt_config *, uint32_t current_generation);
void pt_f439_stop(void);
int pt_f439_capture_start(unsigned psc);
int pt_f439_capture(unsigned *value);
void pt_f439_pins(void);
#endif
