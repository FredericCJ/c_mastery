#include "progress.h"

int tp_donate(const unsigned *base, const int *owner, size_t n, unsigned *out)
{
    if (!base || !owner || !out || !n || n > 4)
        return TP_INVALID;
    unsigned value[4];
    for (size_t i = 0; i < n; i++)
    {
        if (base[i] > 7 || owner[i] < -1 || owner[i] >= (int)n)
            return TP_INVALID;
        value[i] = base[i];
    }
    for (size_t pass = 0; pass < n; pass++)
    {
        for (size_t i = 0; i < n; i++)
            if (owner[i] >= 0 && value[(unsigned)owner[i]] < value[i])
                value[(unsigned)owner[i]] = value[i];
#ifdef TP_MUTANT_DONATION
        break;
#endif
    }
    for (size_t i = 0; i < n; i++)
        out[i] = value[i];
    return TP_OK;
}

int tp_bound(const uint64_t *cost, size_t n, unsigned known, uint64_t deadline,
             uint64_t *out)
{
    if (!cost || !out || !n || n > 8 || known > 1)
        return TP_INVALID;
    if (!known)
        return TP_UNSUPPORTED;
    uint64_t total = 0;
    for (size_t i = 0; i < n; i++)
    {
        if (cost[i] > UINT64_MAX - total)
            return TP_OVERFLOW;
        total += cost[i];
    }
#ifdef TP_MUTANT_BOUND
    total = cost[0];
#endif
    if (total > deadline)
        return TP_UNSUPPORTED;
    *out = total;
    return TP_OK;
}

int tp_nonsleepable(unsigned effects)
{
    if (effects & ~63u)
        return TP_INVALID;
#ifdef TP_MUTANT_EFFECT
    effects &= ~TP_ALLOC;
#endif
    return effects ? TP_UNSUPPORTED : TP_OK;
}
