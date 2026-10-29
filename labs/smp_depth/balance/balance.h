#ifndef SMP_BALANCE_H
#define SMP_BALANCE_H
#include "wakeup.h"

enum
{
    SB_AFFINITY = 10,
    SB_DEFERRED,
    SB_INJECTED
};

struct sb_timer
{
    struct sq_handle task;
    uint64_t serial;
    unsigned old_home;
};

struct sb_world
{
    struct sw_world wake;
    struct sl_lock policy;
    unsigned mask[SQ_TASKS], pending[SQ_TASKS], timer_live[SQ_TASKS];
    uint64_t timer_serial[SQ_TASKS], migrations[SQ_TASKS];
};

/* All operations on the underlying wake/queue world go through this API.
 * One caller per actor; no ISR calls. World/port lifetimes are stable, and
 * output objects are disjoint from the world and one another. Initialization
 * creates an externally distinct lifetime, invalidating ALL previous tokens.
 * Creation/reuse requires whole-world quiescence and no live timer lease.
 * Finish requires the actual context already saved and no longer executing. */
int sb_init(struct sb_world *, unsigned cpus, unsigned capacity,
            struct sw_port);
int sb_create(struct sb_world *, unsigned actor, unsigned home, unsigned slot,
              unsigned mask, struct sq_handle *, struct sq_event *);
int sb_affinity(struct sb_world *, unsigned actor, struct sq_handle,
                unsigned mask, struct sl_stats *);
int sb_dispatch(struct sb_world *, unsigned cpu, struct sq_handle *,
                struct sq_event *, struct sl_stats *);
int sb_finish(struct sb_world *, unsigned cpu, struct sq_handle, unsigned next,
              struct sq_event *, struct sl_stats *);
int sb_wake(struct sb_world *, unsigned actor, struct sq_handle,
            struct sq_event *, struct sl_stats *);
/* fail_at 1 refuses before reservation; 2 performs a real reserve/rollback.
 * On either injected failure ticket/event remain unchanged. A rollback may
 * leave notification retry necessary, as durable source work is retained. */
int sb_move_begin(struct sb_world *, unsigned actor, unsigned destination,
                  struct sq_handle, unsigned fail_at, struct sq_transfer *,
                  struct sq_event *, struct sl_stats *);
int sb_move_end(struct sb_world *, unsigned actor, struct sq_transfer,
                int commit, struct sq_event *, struct sl_stats *);
/* At most one READY migration. Forced disallowed-home repair precedes a
 * load difference >1 heuristic. Return IDLE when no admissible move exists. */
int sb_balance(struct sb_world *, unsigned actor, struct sq_event *begin,
               struct sq_event *end, struct sl_stats *);
int sb_timer_arm(struct sb_world *, unsigned actor, struct sq_handle,
                 struct sb_timer *, struct sl_stats *);
int sb_timer_fire(struct sb_world *, unsigned actor, struct sb_timer,
                  struct sq_event *, struct sl_stats *);
int sb_timer_cancel(struct sb_world *, unsigned actor, struct sb_timer,
                    struct sl_stats *);
int sb_retry(struct sb_world *, unsigned actor, unsigned destination,
             struct sl_stats *);
int sb_prepare_idle(struct sb_world *, unsigned cpu, int masked,
                    struct sl_stats *);
/* Whole-world-quiescent diagnostic, never an unsynchronized worker scan. */
int sb_invariant(const struct sb_world *);
#endif
