#ifndef CACHEABLE_BOOT_MODEL_H
#define CACHEABLE_BOOT_MODEL_H
#include "boot.h"

struct cb_model
{
    uint32_t addresses[96], values[96], count;
    uint32_t memory[8], cache[8], attrs[16], bases[16];
    uint32_t invalid[512][4], selected, sets, writes, line_writes;
    uint32_t dc_writes, enabled_before_invalidation, dirty, release_generation;
    uint32_t release_writes, clean_writes, barriers, instructions, bad;
};

void cb_model_init(struct cb_model *, unsigned sets);
struct cb_io cb_model_io(struct cb_model *);
uint32_t cb_model_read(struct cb_model *, uint32_t);
void cb_model_set(struct cb_model *, uint32_t, uint32_t);
/* Original model, not emulated STM32 cache/peripheral implementation. */
int cb_suite(void);
#endif
