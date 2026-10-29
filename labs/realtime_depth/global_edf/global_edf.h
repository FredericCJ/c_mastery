#ifndef CMASTERY_GLOBAL_EDF_H
#define CMASTERY_GLOBAL_EDF_H
#include "cmx_advanced.h"
#define GE_JOBS 16u
#define GE_TASKS 8u
#define GE_STRANDS 4u
#define GE_HORIZON 256u
#define GE_NONE UINT32_MAX

enum ge_status
{
    GE_OK,
    GE_INVALID,
    GE_UNIMPLEMENTED
};

enum ge_state
{
    GE_FUTURE,
    GE_ACTIVE,
    GE_COMPLETE,
    GE_REJECTED
};

enum ge_kind
{
    GE_IDLE,
    GE_SERVICE,
    GE_QUEUE,
    GE_QUEUE_WAIT,
    GE_MIGRATION
};

struct ge_job
{
    unsigned task, generation, affinity, strands;
    uint32_t nominal, release, jitter, deadline, cost[GE_STRANDS];
    unsigned predecessors[GE_STRANDS];
};

struct ge_config
{
    unsigned cpus, partitioned, home[GE_TASKS], queue_cost, migration_cost,
        horizon;
};

struct ge_job_result
{
    enum ge_state state;
    uint32_t remaining[GE_STRANDS], executed[GE_STRANDS], start, finish;
    unsigned complete_mask, missed;
};

struct ge_tick
{
    enum ge_kind kind;
    uint32_t job, strand, generation, remaining;
};

struct ge_result
{
    struct ge_job_result jobs[GE_JOBS];
    struct ge_tick ticks[GE_HORIZON][2];
    unsigned work, idle, queue, queue_wait, migration_overhead, migrations,
        misses, rejected, completed;
};
/* Externally serialized bounded explicit jobs. INVALID preserves output.
 * Positive strand costs; acyclic predecessor masks; no old job overwrite. */
enum ge_status ge_simulate(const struct ge_job *, unsigned,
                           const struct ge_config *, struct ge_result *);
/* GFB sufficient test only for identical independent implicit-deadline tasks,
 * unrestricted affinities, fully preemptive zero-overhead global EDF. */
enum ge_status ge_sufficient(const cmx_job *, unsigned, unsigned, uint32_t,
                             unsigned *);
#endif
