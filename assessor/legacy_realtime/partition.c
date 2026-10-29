#include "partition.h"
#ifndef RP_MUTANT
#define RP_MUTANT 0
#endif
static int valid(const struct rp_task *t, unsigned n, unsigned cpus, uint32_t h,
                 unsigned demand)
{
    if (!t || !n || n > RP_TASKS || !cpus || cpus > RP_CPUS || !h ||
        h > RP_HORIZON || demand > 1)
        return 0;
    for (unsigned i = 0; i < n; i++)
    {
        const cmx_job *j = &t[i].job;
        if (!j->period || h % j->period || !j->deadline ||
            j->deadline > j->period || j->jitter || j->blocking ||
            (!demand && j->deadline != j->period) || !t[i].affinity ||
            (t[i].affinity >> cpus))
            return 0;
        for (unsigned c = 0; c < cpus; c++)
            if (t[i].cost[c] > j->period)
                return 0;
    }
    return 1;
}

static int fits(const struct rp_task *t, const struct rp_plan *p,
                unsigned count, unsigned demand)
{
    uint32_t used[RP_CPUS] = {0};
    for (unsigned i = 0; i < count; i++)
    {
        unsigned c = p->cpu[i];
        if (c >= p->cpus || (!(t[i].affinity & (1u << c)) && RP_MUTANT != 4))
            return 0;
        uint32_t work = (p->horizon / t[i].job.period) * t[i].cost[c];
        if (work > p->horizon - used[c])
            return 0;
        used[c] += work;
    }
    if (demand && RP_MUTANT != 3)
        for (unsigned c = 0; c < p->cpus; c++)
            for (uint32_t at = 1; at <= p->horizon; at++)
            {
                uint32_t work = 0;
                for (unsigned i = 0; i < count; i++)
                    if (p->cpu[i] == c && at >= t[i].job.deadline)
                    {
                        uint32_t jobs =
                            (at - t[i].job.deadline) / t[i].job.period + 1u;
                        work += jobs * t[i].cost[c];
                    }
                if (work > at)
                    return 0;
            }
    return 1;
}

static void loads(const struct rp_task *t, struct rp_plan *p)
{
    for (unsigned c = 0; c < RP_CPUS; c++)
        p->used[c] = 0;
    for (unsigned i = 0; i < p->n; i++)
        p->used[p->cpu[i]] +=
            (p->horizon / t[i].job.period) * t[i].cost[p->cpu[i]];
}

enum rp_status rp_verify(const struct rp_task *t, unsigned n,
                         const struct rp_plan *p, unsigned demand)
{
    if (!p || p->n != n || !valid(t, n, p->cpus, p->horizon, demand))
        return RP_INVALID;
    if (!fits(t, p, n, demand))
        return RP_IMPOSSIBLE;
    struct rp_plan expected = *p;
    loads(t, &expected);
    for (unsigned c = 0; c < RP_CPUS; c++)
        if (p->used[c] != expected.used[c])
            return RP_INVALID;
    return RP_OK;
}

enum rp_status rp_fit(const struct rp_task *t, unsigned n, unsigned cpus,
                      uint32_t h, unsigned demand, struct rp_plan *out)
{
    if (!out || !valid(t, n, cpus, h, demand))
        return RP_INVALID;
    unsigned order[RP_TASKS];
    uint32_t weight[RP_TASKS];
    for (unsigned i = 0; i < n; i++)
    {
        order[i] = i;
        weight[i] = 0;
        for (unsigned c = 0; c < cpus; c++)
            if (t[i].affinity & (1u << c))
            {
                uint32_t w = (h / t[i].job.period) * t[i].cost[c];
                if (w > weight[i])
                    weight[i] = w;
            }
        unsigned at = i;
        while (RP_MUTANT != 1 && at &&
               weight[order[at]] > weight[order[at - 1]])
        {
            unsigned tmp = order[at];
            order[at] = order[at - 1];
            order[at - 1] = tmp;
            at--;
        }
    }
    struct rp_plan p = {.n = n, .cpus = cpus, .horizon = h};
    unsigned homogeneous = !demand;
    for (unsigned i = 0; i < n; i++)
    {
        if (t[i].affinity != (1u << cpus) - 1u)
            homogeneous = 0;
        for (unsigned c = 1; c < cpus; c++)
            if (t[i].cost[c] != t[i].cost[0])
                homogeneous = 0;
    }
    if (homogeneous)
    {
        cmx_job jobs[RP_TASKS] = {0};
        unsigned placed[RP_TASKS] = {0};
        for (unsigned i = 0; i < n; i++)
        {
            jobs[i] = t[order[i]].job;
            jobs[i].cost = t[order[i]].cost[0];
        }
        enum cmx_result result = cmx_partition(jobs, n, cpus, h, placed);
        if (result != CMX_OK)
        {
            if (RP_MUTANT == 2)
            {
                for (unsigned i = 0; i < n; i++)
                    p.cpu[order[i]] = placed[i];
                *out = p;
            }
            return result == CMX_FULL ? RP_FULL : RP_INVALID;
        }
        for (unsigned i = 0; i < n; i++)
            p.cpu[order[i]] = placed[i];
    }
    else
    {
        struct rp_task sorted[RP_TASKS] = {0};
        struct rp_plan staged = {.n = n, .cpus = cpus, .horizon = h};
        for (unsigned i = 0; i < n; i++)
        {
            sorted[i] = t[order[i]];
            unsigned c;
            for (c = 0; c < cpus; c++)
            {
                staged.cpu[i] = c;
                if (fits(sorted, &staged, i + 1, demand))
                    break;
            }
            if (c == cpus)
            {
                if (RP_MUTANT == 2)
                    *out = staged;
                return RP_FULL;
            }
            p.cpu[order[i]] = c;
        }
    }
    loads(t, &p);
    *out = p;
    return RP_OK;
}

static int search(const struct rp_task *t, struct rp_plan *p, unsigned i,
                  unsigned demand)
{
    if (i == p->n)
        return 1;
    for (unsigned c = 0; c < p->cpus; c++)
    {
        p->cpu[i] = c;
        if (fits(t, p, i + 1, demand) && search(t, p, i + 1, demand))
            return 1;
    }
    return 0;
}

enum rp_status rp_exact(const struct rp_task *t, unsigned n, unsigned cpus,
                        uint32_t h, unsigned demand, struct rp_plan *out)
{
    if (!out || !valid(t, n, cpus, h, demand))
        return RP_INVALID;
    struct rp_plan p = {.n = n, .cpus = cpus, .horizon = h};
    if (!search(t, &p, 0, demand))
        return RP_IMPOSSIBLE;
    loads(t, &p);
    *out = p;
    return RP_OK;
}

enum rp_status rp_simulate(const struct rp_task *t, unsigned n,
                           const struct rp_plan *p, cmx_sim_result *out)
{
    if (!out || rp_verify(t, n, p, 1) != RP_OK)
        return RP_INVALID;
    cmx_job jobs[RP_TASKS] = {0};
    for (unsigned i = 0; i < n; i++)
    {
        jobs[i] = t[i].job;
        jobs[i].cost = t[i].cost[p->cpu[i]];
    }
    cmx_sim_result result;
    enum cmx_result s =
        cmx_simulate(jobs, n, p->cpus, p->cpu, p->horizon, &result);
    if (s != CMX_OK)
        return RP_INVALID;
    *out = result;
    return RP_OK;
}

enum rp_status rp_lease_init(struct rp_lease *p, unsigned owner)
{
    if (!p || owner >= RP_CPUS)
        return RP_INVALID;
    *p = (struct rp_lease){.owner = owner, .target = owner, .epoch = 1};
    return RP_OK;
}

static enum rp_status identity(const struct rp_lease *p, unsigned owner,
                               uint64_t epoch)
{
    if (!p || owner >= RP_CPUS)
        return RP_INVALID;
    return owner != p->owner || epoch != p->epoch ? RP_STALE : RP_OK;
}

enum rp_status rp_begin(struct rp_lease *p, unsigned owner, uint64_t epoch)
{
    enum rp_status s = identity(p, owner, epoch);
    if (s != RP_OK)
        return s;
    if (p->busy || p->prepared)
        return RP_BUSY;
    p->busy = 1;
    return RP_OK;
}

enum rp_status rp_finish(struct rp_lease *p, unsigned owner, uint64_t epoch,
                         uint32_t work)
{
    enum rp_status s = identity(p, owner, epoch);
    if (s != RP_OK)
        return s;
    if (!p->busy)
        return RP_INVALID;
    if (work > UINT64_MAX - p->work)
        return RP_FULL;
    p->work += work;
    p->busy = 0;
    return RP_OK;
}

enum rp_status rp_prepare(struct rp_lease *p, unsigned owner, uint64_t epoch,
                          unsigned target)
{
    enum rp_status s = identity(p, owner, epoch);
    if (s != RP_OK)
        return s;
    if (target >= RP_CPUS || target == owner)
        return RP_INVALID;
    if ((p->busy && RP_MUTANT != 5) || p->prepared)
        return RP_BUSY;
    if (p->epoch == UINT64_MAX)
        return RP_FULL;
    p->prepared = 1;
    p->target = target;
    return RP_OK;
}

enum rp_status rp_accept(struct rp_lease *p, unsigned target, uint64_t epoch)
{
    if (!p || target >= RP_CPUS)
        return RP_INVALID;
    if (!p->prepared || target != p->target || epoch != p->epoch)
        return RP_STALE;
    if (p->busy)
        return RP_BUSY;
    p->owner = target;
    if (RP_MUTANT != 6)
        p->epoch++;
    p->prepared = 0;
    return RP_OK;
}

enum rp_status rp_abort(struct rp_lease *p, unsigned owner, uint64_t epoch)
{
    enum rp_status s = identity(p, owner, epoch);
    if (s != RP_OK)
        return s;
    if (!p->prepared)
        return RP_INVALID;
    p->prepared = 0;
    p->target = owner;
    return RP_OK;
}
