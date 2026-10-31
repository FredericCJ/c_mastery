#include "progress.h"

enum
{
    MAX_THREADS = 4,
    MAX_PRIORITY = 7,
    MAX_PHASES = 8,
    /* Every effect bit the contract defines; any other bit is invalid. */
    KNOWN_EFFECTS =
        TP_ALLOC | TP_PAGEFAULT | TP_LOG | TP_BLOCK | TP_PREEMPT | TP_UNAUDITED
};

/*
 * Priority inheritance over a wait-for graph: owner[i] is the thread that
 * thread i waits on (-1 when it waits on nobody). Each thread's effective
 * priority is the maximum base priority among itself and every thread that
 * waits on it directly or through a chain. n passes suffice because a chain
 * has at most n - 1 edges. *out is written only on TP_OK; callers recompute
 * from base priorities after a withdrawal and never feed `out` back in.
 */
int tp_donate(const unsigned *base, const int *owner, size_t n, unsigned *out)
{
    if (!base || !owner || !out || !n || n > MAX_THREADS)
    {
        return TP_INVALID;
    }
    unsigned value[MAX_THREADS];
    for (size_t i = 0; i < n; i++)
    {
        if (base[i] > MAX_PRIORITY || owner[i] < -1 || owner[i] >= (int)n)
        {
            return TP_INVALID;
        }
        value[i] = base[i];
    }
    for (size_t pass = 0; pass < n; pass++)
    {
        for (size_t i = 0; i < n; i++)
        {
            if (owner[i] >= 0 && value[(unsigned)owner[i]] < value[i])
            {
                value[(unsigned)owner[i]] = value[i];
            }
        }
#ifdef TP_MUTANT_DONATION
        break;
#endif
    }
    for (size_t i = 0; i < n; i++)
    {
        out[i] = value[i];
    }
    return TP_OK;
}

/*
 * Conditional response-time bound: the sum of the n phase costs, valid only
 * when the delays are known (known == 1) and the sum fits the deadline.
 * TP_UNSUPPORTED for unknown delays or a bound above deadline, TP_OVERFLOW
 * when the sum would wrap. *out is written only on TP_OK.
 */
int tp_bound(const uint64_t *cost, size_t n, unsigned known, uint64_t deadline,
             uint64_t *out)
{
    if (!cost || !out || !n || n > MAX_PHASES || known > 1)
    {
        return TP_INVALID;
    }
    if (!known)
    {
        return TP_UNSUPPORTED;
    }
    uint64_t total = 0;
    for (size_t i = 0; i < n; i++)
    {
        if (cost[i] > UINT64_MAX - total)
        {
            return TP_OVERFLOW;
        }
        total += cost[i];
    }
#ifdef TP_MUTANT_BOUND
    total = cost[0];
#endif
    if (total > deadline)
    {
        return TP_UNSUPPORTED;
    }
    *out = total;
    return TP_OK;
}

/* TP_OK only for an empty effect set: every named effect may sleep, fault
 * or take unbounded time, so any one of them is TP_UNSUPPORTED. */
int tp_nonsleepable(unsigned effects)
{
    if (effects & ~(unsigned)KNOWN_EFFECTS)
    {
        return TP_INVALID;
    }
#ifdef TP_MUTANT_EFFECT
    effects &= ~TP_ALLOC;
#endif
    return effects ? TP_UNSUPPORTED : TP_OK;
}
