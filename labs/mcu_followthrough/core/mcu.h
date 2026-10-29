#ifndef MCU_FOLLOWTHROUGH_H
#define MCU_FOLLOWTHROUGH_H
#include "cmx.h"
/* All calls are serialized by one saved-mask domain. Pointer validity and
 * storage lifetime are caller premises. Slot0 is permanent idle. Public
 * handles contain all32 generation bits and a1-based five-bit slot field.
 * Initial frame is the exact integer PendSV frame, not a cooperative frame.
 * A port updates executing only after a physical stack handoff. */
typedef uint64_t mf_handle_id;

struct mf_mf
{
    cmx_kernel kernel;
    uint32_t *sp[CMX_TASKS];
    unsigned detached[CMX_TASKS], consumed[CMX_TASKS];
    unsigned faulted[CMX_TASKS], fault_valid[CMX_TASKS];
    uint32_t fault_code[CMX_TASKS], fault_pc[CMX_TASKS];
    unsigned fatal;
    int executing;
};
enum cmx_result mf_init(struct mf_mf *, uint32_t *, size_t, enum cmx_policy);
enum cmx_result mf_add(struct mf_mf *, uint32_t *, size_t, uint32_t entry,
                       uint32_t return_pc, unsigned priority, unsigned detached,
                       unsigned admission_budget, mf_handle_id *);
int mf_lookup(const struct mf_mf *, mf_handle_id);
mf_handle_id mf_handle(const struct mf_mf *, unsigned);
int mf_schedule(struct mf_mf *);
int mf_schedule_reverse(struct mf_mf *);
enum cmx_result mf_exit(struct mf_mf *, mf_handle_id, int);
enum cmx_result mf_join(struct mf_mf *, mf_handle_id, mf_handle_id,
                        cmx_deadline, int *);
enum cmx_result mf_join_result(struct mf_mf *, mf_handle_id, mf_handle_id,
                               int *);
enum cmx_result mf_reap(struct mf_mf *, mf_handle_id);
enum cmx_result mf_fault(struct mf_mf *, mf_handle_id, uint32_t code,
                         uint32_t pc);
int mf_invariant(const struct mf_mf *);

/* Timer ownership is independent of task identity. Expiry delivery includes
 * the generation ticket; cancellation/reuse cannot turn an old ticket into
 * a new event. at-now and each poll delta must be below half-range. */
#define MF_TIMERS 16u

struct mf_timer
{
    uint32_t at, generation;
    uint64_t sequence;
    unsigned active, retired;
};

struct mf_timers
{
    struct mf_timer timer[MF_TIMERS];
    uint32_t now;
    uint64_t next_sequence;
};

void mf_timers_init(struct mf_timers *, uint32_t now);
enum cmx_result mf_timer_arm(struct mf_timers *, uint32_t at, mf_handle_id *);
enum cmx_result mf_timer_cancel(struct mf_timers *, mf_handle_id);
enum cmx_result mf_timer_poll(struct mf_timers *, uint32_t now, mf_handle_id *,
                              size_t *);
int mf_due32(uint32_t now, uint32_t at);
int mf_due64(uint64_t now, uint64_t at);
enum cmx_result mf_periodic(uint32_t phase, uint32_t period, uint32_t now,
                            uint32_t *next, uint32_t *skipped);
uint64_t mf_div64(uint64_t numerator, uint64_t denominator);
enum cmx_result mf_counts(uint64_t us, uint32_t hz, uint32_t maximum,
                          uint32_t *counts, uint64_t *remaining_us);
enum cmx_result mf_basepri(unsigned incoming, unsigned requested,
                           unsigned implemented_bits, unsigned *effective);

/* Deterministic sleep protocol, independently of the target timer. A wake
 * sets ready/event before sleep; time compensation consumes one arm exactly
 * once. This models a continuously running counter, not an asleep timer. */
struct mf_sleep
{
    uint32_t start, counts, elapsed;
    unsigned armed, ready, event;
    unsigned slept, credited;
};

void mf_sleep_init(struct mf_sleep *);
enum cmx_result mf_sleep_arm(struct mf_sleep *, uint32_t start,
                             uint32_t counts);
void mf_sleep_wake(struct mf_sleep *);
int mf_sleep_enter(struct mf_sleep *);
enum cmx_result mf_sleep_finish(struct mf_sleep *, uint32_t now);
#endif
