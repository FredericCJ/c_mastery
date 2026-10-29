#ifndef MF_SLEEP_TARGET_H
#define MF_SLEEP_TARGET_H
#include "target.h"
/* QEMU AN386's continuously clocked25MHz CMSDK timer0 is the elapsed source.
 * Timer1 is a one-shot wake source. Real deep-sleep clock gates are outside
 * this adapter and must be supplied as a different target contract. */
struct mf_nap { uint32_t start,requested,elapsed,remaining;
                 unsigned armed,credited,slept,wake_irq; };
void mf_clock_init(void);
uint32_t mf_clock_now(void);
void mf_nap(struct mf_nap *,uint32_t,unsigned boundary);
int mf_nap_finish(struct mf_nap *);
extern volatile uint32_t mf_sleep_irqs,mf_sleep_early;
#endif
