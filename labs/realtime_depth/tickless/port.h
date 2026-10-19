#ifndef CMASTERY_TICKLESS_PORT_H
#define CMASTERY_TICKLESS_PORT_H
#include "tickless.h"
typedef struct { TQTimer *timer;uint32_t raw_origin,logical_origin;
 TQResume resume;volatile unsigned interrupts;unsigned stopped; } TPClock;
/* stopped:0 running,1 auxiliary-clock resume pending,2 shut down. Only init
 * leaves state2. No queue/tick advancement is admitted in state1 or state2. */
/* Owned QEMU MPS2-AN386 only. Owns CMSDK timer0 and SysTick. Clock counts,
 * not physical elapsed-time or a STM32 power/oscillator guarantee. */
enum tq_status tp_init(TPClock *,TQTimer *,uint32_t logical_origin);
uint32_t tp_raw(void);
uint32_t tp_now(const TPClock *);
enum tq_status tp_wait(TPClock *,unsigned,uint32_t,unsigned);
enum tq_status tp_at(TPClock *,unsigned,uint32_t);
enum tq_status tp_arm(TPClock *,uint32_t injected_masked_delay);
enum tq_status tp_interrupt(TPClock *);
enum tq_status tp_stop(TPClock *);
enum tq_status tp_resume(TPClock *);
void tp_shutdown(TPClock *);
#endif
