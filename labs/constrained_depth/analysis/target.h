#ifndef CMASTERY_CONSTRAINED_ANALYSIS_TARGET_H
#define CMASTERY_CONSTRAINED_ANALYSIS_TARGET_H
#include <stdint.h>
extern volatile uint32_t ca_ticks,ca_ready,ca_result,ca_fault;
uint32_t ca_div32(uint32_t,uint32_t);
uint64_t ca_div64(uint64_t,uint64_t);
unsigned ca_target_init(uint32_t);
unsigned ca_target_paths(void);
#endif
