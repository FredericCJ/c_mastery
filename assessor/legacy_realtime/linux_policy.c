#include "policy.h"
#ifndef LC_MUTANT
#define LC_MUTANT 0
#endif
int lc_select(const struct lc_entity *entities, size_t count, size_t *selected)
{
    if (!entities || !selected || !count || count > 8)
        return -1;
    size_t best = count;
    for (size_t i = 0; i < count; ++i)
    {
        if (LC_MUTANT != 1 && entities[i].lag < 0)
            continue;
        if (best == count ||
            entities[i].virtual_deadline < entities[best].virtual_deadline)
            best = i;
    }
    if (best == count)
        return 0;
    *selected = best;
    return 1;
}

int lc_parameters(uint64_t runtime, uint64_t deadline, uint64_t period)
{
    return runtime > 0 && (LC_MUTANT == 2 || runtime <= deadline) &&
           deadline <= period;
}
