#ifndef CMASTERY_OVERLOAD_H
#define CMASTERY_OVERLOAD_H
#include <stdint.h>
#define OD_TASKS 4u
#define OD_REQUESTS 96u
#define OD_TICKS 128u
#define OD_NONE UINT32_MAX

enum od_status
{
    OD_OK,
    OD_INVALID,
    OD_RESOURCE,
    OD_UNIMPLEMENTED
};

enum od_decision
{
    OD_ADMITTED,
    OD_REJECTED,
    OD_INCONCLUSIVE
};

enum od_policy
{
    OD_REJECT_NEW,
    OD_DROP_OLDEST,
    OD_LATEST
};

enum od_state
{
    OD_FUTURE,
    OD_PENDING,
    OD_DONE,
    OD_REJECT,
    OD_DROP,
    OD_MISS,
    OD_OVERRUN,
    OD_MODE_DROP,
    OD_MODE_REJECT
};

struct od_task
{
    uint32_t cost, period;
    unsigned critical;
};

struct od_set
{
    struct od_task tasks[OD_TASKS];
    unsigned count;
};

struct od_request
{
    uint32_t id, stream, release, cost, deadline, io_until;
};

struct od_item
{
    enum od_state state;
    uint32_t executed, finish;
};

struct od_tick
{
    uint32_t selected, pending, mode, heartbeat;
};

struct od_result
{
    struct od_item items[OD_REQUESTS];
    struct od_tick ticks[OD_TICKS];
    unsigned peak, heartbeats, work, idle, mode_switches, recovered,
        recovery_refused;
};

struct od_dependency
{
    unsigned producer_waiting, consumer_waiting, mutex_owner, repaired_owner,
        space, producer_result;
};
/* Live disjoint storage. INVALID/RESOURCE leave all outputs unchanged.
 * Candidate installation occurs only after a complete empty-boundary cyclic
 * proof under the exact implicit-deadline, fixed-supervisor-slot model.
 * REJECTED/INCONCLUSIVE preserve the set. No finite-prefix acceptance. */
enum od_status od_admit(struct od_set *, struct od_task, unsigned horizon,
                        enum od_decision *);
/* Immutable unique request IDs; copied data with no external borrowed I/O.
 * io_until is an externally supplied eligibility boundary, not an I/O driver.
 * supervisor_period is 0 (queue-only comparison) or4 (reserved slot0).
 * switch_at/recover_at may be OD_NONE. Recovery requires an empty live queue.
 * Overrun drops a request after its task budget, retaining consumed work.
 */
enum od_status od_run(const struct od_set *, const struct od_request *,
                      unsigned requests, unsigned capacity, unsigned horizon,
                      enum od_policy, unsigned supervisor_period,
                      uint32_t switch_at, uint32_t recover_at,
                      struct od_result *);
/* Actual completed CMX mutex/semaphore cycle and cancellation/handoff repair. */
enum od_status od_backpressure(struct od_dependency *);
#endif
