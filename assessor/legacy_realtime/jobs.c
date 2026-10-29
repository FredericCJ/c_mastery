#include "jobs.h"
#include "cmx_advanced.h"
#ifndef SJ_MUTANT
#define SJ_MUTANT 0
#endif
static int valid(const struct sj_task *t, unsigned n)
{
    if (!t || !n || n > SJ_TASKS)
    {
        return 0;
    }
    for (unsigned i = 0; i < n; i++)
    {
        if (!t[i].period || !t[i].deadline || t[i].cost > 1024u)
        {
            return 0;
        }
    }
    return 1;
}

enum sj_status sj_order(const struct sj_task *t, unsigned n, unsigned dm,
                        unsigned *out)
{
    cmx_job j[SJ_TASKS];
    unsigned order[SJ_TASKS];
    if (SJ_MUTANT == 7 && out)
    {
        out[0] = 0;
    }
    if (!valid(t, n) || dm > 1 || !out)
    {
        return SJ_INVALID;
    }
    for (unsigned i = 0; i < n; i++)
    {
        j[i] = (cmx_job){t[i].cost, t[i].period, t[i].deadline, 0, 0};
    }
    if (cmx_priority_order(j, n, SJ_MUTANT == 6 ? !dm : dm, order) != CMX_OK)
    {
        return SJ_RESOURCE;
    }
    for (unsigned i = 0; i < n; i++)
    {
        out[i] = order[i];
    }
    return SJ_OK;
}

enum sj_status sj_generate(const struct sj_task *t, unsigned n,
                           const uint32_t *phase, const uint32_t *jitter,
                           unsigned k, struct sj_job *out, unsigned capacity,
                           unsigned *count)
{
    struct sj_job tmp[SJ_JOBS];
    if (!valid(t, n) || !phase || !count || !out || (!jitter && k) ||
        k > SJ_JOBS || capacity > SJ_JOBS)
    {
        return SJ_INVALID;
    }
    if (k > SJ_JOBS / n || k * n > capacity)
    {
        return SJ_RESOURCE;
    }
    for (unsigned j = 0; j < k; j++)
    {
        for (unsigned i = 0; i < n; i++)
        {
            unsigned at = j * n + i;
            uint32_t delay = jitter[at];
            if (delay > t[i].jitter)
            {
                return SJ_INVALID;
            }
            if (SJ_MUTANT == 9)
            {
                delay = 0;
            }
            if (j && t[i].period > UINT32_MAX / j)
            {
                return SJ_RESOURCE;
            }
            uint32_t r = j * t[i].period;
            if (phase[i] > UINT32_MAX - r)
            {
                return SJ_RESOURCE;
            }
            r += phase[i];
            if (delay > UINT32_MAX - r)
            {
                return SJ_RESOURCE;
            }
            r += delay;
            if (t[i].deadline > UINT32_MAX - r)
            {
                return SJ_RESOURCE;
            }
            tmp[at] = (struct sj_job){i, j, r, t[i].cost, 0, 0};
        }
    }
    for (unsigned j = 0; j < k * n; j++)
    {
        out[j] = tmp[j];
    }
    *count = k * n;
    return SJ_OK;
}

static int locked(const struct sj_job *j, const struct sj_result *r,
                  unsigned current)
{
    return current != SJ_NONE && r->jobs[current].state == SJ_READY &&
           r->jobs[current].executed >= j[current].nonpreempt_start &&
           r->jobs[current].executed - j[current].nonpreempt_start <
               j[current].nonpreempt_length;
}

static int earlier(const struct sj_task *t, const struct sj_job *j,
                   const unsigned *rank, unsigned a, unsigned b,
                   enum sj_policy p)
{
    uint32_t ka =
        p == SJ_EDF ? j[a].release + t[j[a].task].deadline : rank[j[a].task];
    uint32_t kb =
        p == SJ_EDF ? j[b].release + t[j[b].task].deadline : rank[j[b].task];
    if (SJ_MUTANT == 1 && p == SJ_EDF)
    {
        ka = j[a].release;
        kb = j[b].release;
    }
    if (ka != kb)
    {
        return ka < kb;
    }
    if (j[a].release != j[b].release)
    {
        return j[a].release < j[b].release;
    }
    return SJ_MUTANT == 10 ? a > b : a < b;
}

enum sj_status sj_simulate(const struct sj_task *t, unsigned n,
                           const struct sj_job *j, unsigned k,
                           enum sj_policy policy, enum sj_overload overload,
                           unsigned capacity, unsigned horizon, unsigned cost,
                           struct sj_result *out)
{
    struct sj_result r = {0};
    unsigned order[SJ_TASKS], rank[SJ_TASKS], current = SJ_NONE, active = 0,
                                              overhead = 0;
    if (!valid(t, n) || (!j && k) || k > SJ_JOBS || (unsigned)policy > SJ_EDF ||
        (unsigned)overload > SJ_DROP_OLDEST || !capacity ||
        capacity > SJ_JOBS || !horizon || horizon > SJ_HORIZON ||
        cost > SJ_HORIZON || !out)
    {
        return SJ_INVALID;
    }
    if (sj_order(t, n, policy == SJ_DM, order) != SJ_OK)
    {
        return SJ_RESOURCE;
    }
    for (unsigned i = 0; i < n; i++)
    {
        rank[order[i]] = i;
    }
    for (unsigned i = 0; i < k; i++)
    {
        if (j[i].task >= n || j[i].cost > 1024u ||
            j[i].nonpreempt_start > j[i].cost ||
            j[i].nonpreempt_length > j[i].cost - j[i].nonpreempt_start ||
            t[j[i].task].deadline > UINT32_MAX - j[i].release)
        {
            return SJ_INVALID;
        }
        for (unsigned q = 0; q < i; q++)
        {
            if (j[q].task == j[i].task && j[q].sequence == j[i].sequence)
            {
                return SJ_INVALID;
            }
        }
        r.jobs[i] = (struct sj_job_result){SJ_FUTURE, j[i].cost, 0,
                                           SJ_NONE,   SJ_NONE,   0};
    }
    for (unsigned tick = 0; tick <= horizon; tick++)
    {
        for (unsigned i = 0; i < k; i++)
        {
            if (r.jobs[i].state == SJ_READY && !r.jobs[i].missed &&
                j[i].release + t[j[i].task].deadline == tick && SJ_MUTANT != 2)
            {
                r.jobs[i].missed = 1;
                r.misses++;
            }
        }
        if (tick == horizon)
        {
            break;
        }
        r.ticks[tick] = (struct sj_tick){SJ_NONE, SJ_IDLE};
        for (unsigned i = 0; i < k; i++)
        {
            if (j[i].release == tick)
            {
                if (!j[i].cost)
                {
                    r.jobs[i].state = SJ_COMPLETED;
                    r.jobs[i].start = tick;
                    r.jobs[i].finish = tick;
                    r.completed++;
                    continue;
                }
                if (SJ_MUTANT == 3)
                {
                    for (unsigned q = 0; q < k; q++)
                    {
                        if (r.jobs[q].state == SJ_READY &&
                            j[q].task == j[i].task)
                        {
                            r.jobs[q].state = SJ_DROPPED;
                            r.dropped++;
                            active--;
                            if (current == q)
                            {
                                current = SJ_NONE;
                            }
                        }
                    }
                }
                if (active == capacity && SJ_MUTANT != 8)
                {
                    unsigned victim = SJ_NONE;
                    if (overload == SJ_DROP_OLDEST)
                    {
                        for (unsigned q = 0; q < k; q++)
                        {
                            if (r.jobs[q].state == SJ_READY &&
                                !(q == current && locked(j, &r, current)) &&
                                (victim == SJ_NONE ||
                                 j[q].release < j[victim].release ||
                                 (j[q].release == j[victim].release &&
                                  q < victim)))
                            {
                                victim = q;
                            }
                        }
                    }
                    if (victim == SJ_NONE)
                    {
                        r.jobs[i].state = SJ_REJECTED;
                        r.rejected++;
                        continue;
                    }
                    r.jobs[victim].state = SJ_DROPPED;
                    r.dropped++;
                    active--;
                    if (current == victim)
                    {
                        current = SJ_NONE;
                    }
                }
                r.jobs[i].state = SJ_READY;
                active++;
                if (active > r.peak_active)
                {
                    r.peak_active = active;
                }
            }
        }
        if (overhead)
        {
            overhead--;
            r.overhead++;
            r.ticks[tick].kind = SJ_OVERHEAD;
            continue;
        }
        unsigned best = SJ_NONE;
        for (unsigned i = 0; i < k; i++)
        {
            if (r.jobs[i].state == SJ_READY &&
                (best == SJ_NONE || earlier(t, j, rank, i, best, policy)))
            {
                best = i;
            }
        }
        if (SJ_MUTANT != 4 && locked(j, &r, current))
        {
            best = current;
        }
        if (current != SJ_NONE && best != current)
        {
            r.preemptions++;
            current = SJ_NONE;
            if (cost && SJ_MUTANT != 5)
            {
                overhead = cost - 1;
                r.overhead++;
                r.ticks[tick].kind = SJ_OVERHEAD;
                continue;
            }
        }
        if (best == SJ_NONE)
        {
            r.idle++;
            continue;
        }
        current = best;
        r.ticks[tick] = (struct sj_tick){best, SJ_SERVICE};
        r.work++;
        struct sj_job_result *b = &r.jobs[best];
        if (b->start == SJ_NONE)
        {
            b->start = tick;
        }
        b->executed++;
        b->remaining--;
        if (!b->remaining)
        {
            b->state = SJ_COMPLETED;
            b->finish = tick + 1;
            r.completed++;
            active--;
            current = SJ_NONE;
        }
    }
    r.elapsed = horizon;
    *out = r;
    return SJ_OK;
}

static uint32_t gcd(uint32_t a, uint32_t b)
{
    while (b)
    {
        uint32_t c = a % b;
        a = b;
        b = c;
    }
    return a;
}

static int mul(uint64_t a, uint64_t b, uint64_t *out)
{
    uint64_t result = 0;
    while (b)
    {
        if (b & 1u)
        {
            if (a > UINT64_MAX - result)
            {
                return 0;
            }
            result += a;
        }
        b >>= 1;
        if (b)
        {
            if (a > UINT64_MAX - a)
            {
                return 0;
            }
            a += a;
        }
    }
    *out = result;
    return 1;
}

enum sj_status sj_rm_bound(const struct sj_task *t, unsigned n, unsigned *out)
{
    uint32_t h = 1;
    uint64_t numerator = 0, a, b, x = 1, y = 1;
    if (!valid(t, n) || !out)
    {
        return SJ_INVALID;
    }
    for (unsigned i = 0; i < n; i++)
    {
        if (t[i].deadline != t[i].period || t[i].jitter)
        {
            return SJ_INVALID;
        }
        uint32_t q = t[i].period / gcd(h, t[i].period);
        if (q > 65535u / h)
        {
            return SJ_RESOURCE;
        }
        h *= q;
    }
    for (unsigned i = 0; i < n; i++)
    {
        numerator += (uint64_t)t[i].cost * (h / t[i].period);
    }
    b = (uint64_t)n * h;
    a = b + numerator;
    for (unsigned i = 0; i < n; i++)
    {
        if (!mul(x, a, &x) || !mul(y, b, &y))
        {
            return SJ_RESOURCE;
        }
    }
    if (y > UINT64_MAX / 2u)
    {
        return SJ_RESOURCE;
    }
    *out = SJ_MUTANT == 11 ? 1u : (unsigned)(x <= 2u * y);
    return SJ_OK;
}
