#ifndef SMP_BALANCE_H
#define SMP_BALANCE_H
#include "wakeup.h"
enum { SB_AFFINITY=10,SB_DEFERRED,SB_INJECTED };
typedef struct {SqHandle task;uint64_t serial;unsigned old_home;} SbTimer;
typedef struct {SwWorld wake;SlLock policy;unsigned mask[SQ_TASKS],pending[SQ_TASKS],timer_live[SQ_TASKS];uint64_t timer_serial[SQ_TASKS],migrations[SQ_TASKS];} SbWorld;
/* All operations on the underlying wake/queue world go through this API.
 * One caller per actor; no ISR calls. World/port lifetimes are stable, and
 * output objects are disjoint from the world and one another. Initialization
 * creates an externally distinct lifetime, invalidating ALL previous tokens.
 * Creation/reuse requires whole-world quiescence and no live timer lease.
 * Finish requires the actual context already saved and no longer executing. */
int sb_init(SbWorld *,unsigned cpus,unsigned capacity,SwPort);
int sb_create(SbWorld *,unsigned actor,unsigned home,unsigned slot,unsigned mask,SqHandle *,SqEvent *);
int sb_affinity(SbWorld *,unsigned actor,SqHandle,unsigned mask,SlStats *);
int sb_dispatch(SbWorld *,unsigned cpu,SqHandle *,SqEvent *,SlStats *);
int sb_finish(SbWorld *,unsigned cpu,SqHandle,unsigned next,SqEvent *,SlStats *);
int sb_wake(SbWorld *,unsigned actor,SqHandle,SqEvent *,SlStats *);
/* fail_at 1 refuses before reservation; 2 performs a real reserve/rollback.
 * On either injected failure ticket/event remain unchanged. A rollback may
 * leave notification retry necessary, as durable source work is retained. */
int sb_move_begin(SbWorld *,unsigned actor,unsigned destination,SqHandle,unsigned fail_at,SqTransfer *,SqEvent *,SlStats *);
int sb_move_end(SbWorld *,unsigned actor,SqTransfer,int commit,SqEvent *,SlStats *);
/* At most one READY migration. Forced disallowed-home repair precedes a
 * load difference >1 heuristic. Return IDLE when no admissible move exists. */
int sb_balance(SbWorld *,unsigned actor,SqEvent *begin,SqEvent *end,SlStats *);
int sb_timer_arm(SbWorld *,unsigned actor,SqHandle,SbTimer *,SlStats *);
int sb_timer_fire(SbWorld *,unsigned actor,SbTimer,SqEvent *,SlStats *);
int sb_timer_cancel(SbWorld *,unsigned actor,SbTimer,SlStats *);
int sb_retry(SbWorld *,unsigned actor,unsigned destination,SlStats *);
int sb_prepare_idle(SbWorld *,unsigned cpu,int masked,SlStats *);
/* Whole-world-quiescent diagnostic, never an unsynchronized worker scan. */
int sb_invariant(const SbWorld *);
#endif
