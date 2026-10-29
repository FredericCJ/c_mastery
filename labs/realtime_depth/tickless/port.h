#ifndef CMASTERY_TICKLESS_PORT_H
#define CMASTERY_TICKLESS_PORT_H
#include "tickless.h"

struct tp_clock
{
    struct tq_timer *timer;
    uint32_t raw_origin, logical_origin;
    struct tq_resume resume;
    volatile unsigned interrupts;
    unsigned stopped;
};
/* stopped:0 running,1 auxiliary-clock resume pending,2 shut down. Only init
 * leaves state2. No queue/tick advancement is admitted in state1 or state2. */
/* Owned QEMU MPS2-AN386 only. Owns CMSDK timer0 and SysTick. Clock counts,
 * not physical elapsed-time or a STM32 power/oscillator guarantee. */
enum tq_status tp_init(struct tp_clock *, struct tq_timer *,
                       uint32_t logical_origin);
uint32_t tp_raw(void);
uint32_t tp_now(const struct tp_clock *);
enum tq_status tp_wait(struct tp_clock *, unsigned, uint32_t, unsigned);
enum tq_status tp_at(struct tp_clock *, unsigned, uint32_t);
enum tq_status tp_arm(struct tp_clock *, uint32_t injected_masked_delay);
enum tq_status tp_interrupt(struct tp_clock *);
enum tq_status tp_stop(struct tp_clock *);
enum tq_status tp_resume(struct tp_clock *);
void tp_shutdown(struct tp_clock *);
#endif
