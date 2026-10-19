#ifndef CMASTERY_CONSTRAINED_POWER_H
#define CMASTERY_CONSTRAINED_POWER_H
#include "supervision.h"
enum cs_reg {CS_RCC_CR,CS_RCC_CFGR,CS_RCC_APB1ENR,CS_PWR_CR,CS_IWDG_KR,CS_IWDG_PR,CS_IWDG_RLR,CS_IWDG_SR,CS_SCR,CS_DBG,CS_SYSTICK,CS_REG_COUNT};
typedef struct {void *context;uint32_t(*read)(void*,enum cs_reg);void(*write)(void*,enum cs_reg,uint32_t);void(*idle)(void*);} CSBus;
typedef struct {unsigned epoch,phase,elapsed_known;uint64_t elapsed;} CSStop;
/* Fresh stable storage. phase0=active,1=clock recovery required,2=complete.
 * A new epoch is published even when an external wake has no retained clock. */
enum cs_status cs_stop_init(CSStop *);
/* Caller owns all registers and has masked ordinary interrupts. All five
 * quiescence premises (DMA, flash, APB, output, wake configuration) are true
 * iff premises==31. Selected F439 HSI16/noPLL/normal regulator/noDebug profile.
 * saved_clock/after_clock are trusted retained-clock samples, or known==0.
 * Valid live disjoint objects; callbacks do not fail/reenter or mutate state. */
enum cs_status cs_stop_enter(CSStop *,const CSBus *,unsigned premises);
enum cs_status cs_stop_finish(CSStop *,const CSBus *,unsigned known,uint64_t saved_clock,uint64_t after_clock);
/* Irreversible watchdog start: WAIT after start leaves watchdog running.
 * budget counts polls, never a physical duration. divisor encoding0..6. */
enum cs_status cs_iwdg_start(const CSBus *,unsigned divisor,unsigned reload,unsigned budget);
enum cs_status cs_iwdg_feed(CSWorld *,const CSBus *);
/* Compile-only selected hardware binding. No board is flashed by the runner.
 * Stop is WFI; wake source/retained clock must be configured by caller. */
enum cs_status cs_f439_stop(CSStop *,unsigned premises);
CSBus cs_f439_bus(void);
#endif
