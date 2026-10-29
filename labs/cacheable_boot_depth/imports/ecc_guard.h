#ifndef B06_ECC_GUARD_H
#define B06_ECC_GUARD_H
#include <stdint.h>
/* Recover only an exact basic-frame precise data fault at the admitted load.
 * All other faults remain fatal. Output frame unchanged on rejection. */
int eg_frame(uint32_t frame[8],uint32_t exception_return,uint32_t cfsr,uint32_t bfar,
             uint32_t load_pc,uint32_t abort_pc,uint32_t admitted_address);
typedef int (*eg_read_fn)(void *,uint32_t,uint32_t *);
int eg_snapshot(eg_read_fn,void *,uint32_t base,unsigned char out[128]);
/* Faulting candidate is unavailable; only readable valid records participate.
 * Result0 none,1 first,2 second,-1 equal-sequence conflict; output unchanged
 * for0/-1. Snapshot faults do not authorize retry/reprogram in place. */
int eg_scan_pair(eg_read_fn,void *,uint32_t first,uint32_t second,unsigned char out[128]);
/* Privileged startup-only, no concurrency, no live interrupts/FP frame;
 * BusFault enabled at an eligible priority. Actual handler must be installed. */
int eg_read_word(uint32_t address,uint32_t *out);
void eg_busfault(void);
#endif
