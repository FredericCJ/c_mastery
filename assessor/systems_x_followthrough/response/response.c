#include "response.h"
#ifndef SX_RESPONSE_MUTANT
#define SX_RESPONSE_MUTANT 0
#endif
int sx_switch_response(uint32_t own, uint32_t blocking, uint32_t deadline,
                       const struct b5_task *h, size_t n, uint32_t cost,
                       uint32_t *out)
{
    struct b5_task charged[4];
    uint64_t overhead = (uint64_t)cost * 2, base = (uint64_t)own + overhead;
    if (!out || (!h && n) || n > 4 || !own || !deadline || deadline > 1000000)
        return B5_INVALID;
    for (size_t i = 0; i < n; i++)
        if (!h[i].period || h[i].cost > 1000000 || h[i].jitter > 1000000)
            return B5_INVALID;
    if (base > UINT32_MAX || overhead > 1000000)
        return B5_RANGE;
    for (size_t i = 0; i < n; i++)
    {
        uint64_t c = (uint64_t)h[i].cost + overhead;
        if (c > 1000000)
            return B5_RANGE;
        charged[i] = h[i];
        charged[i].cost = (uint32_t)c;
    }
#if SX_RESPONSE_MUTANT == 1
    for (size_t i = 0; i < n; i++)
        charged[i].cost = h[i].cost;
#elif SX_RESPONSE_MUTANT == 2
    base += overhead;
    if (base > UINT32_MAX)
        return B5_RANGE;
#endif
    return b5_response((uint32_t)base, blocking, deadline, charged, n, out);
}
