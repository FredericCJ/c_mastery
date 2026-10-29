#include "atomics.h"

int ta_litmus(unsigned litmus, unsigned model, unsigned first, unsigned second)
{
    if (litmus > TA_SB || model > TA_MODEL_MIXED || first > 1 || second > 1)
    {
        return -1;
    }
    if (model == TA_MODEL_RELAXED || model == TA_MODEL_MIXED)
    {
        return 1;
    }
    if (model == TA_MODEL_RA)
    {
#ifdef TA_MUTANT_MODEL
        return litmus == TA_MP ? !(first && !second) : (first || second);
#else
        return litmus != TA_MP || !first || second;
#endif
    }
    /* All four operations SC: total order extends SB, RF and from-read.
       Events0,1 are thread0; events2,3 are thread1. Initial stores precede all.
     */
    unsigned edges[4] = {2, 0, 8, 0};
    unsigned reads[2] = {litmus == TA_MP ? 2u : 1u, 3u};
    unsigned writes[2] = {litmus == TA_MP ? 1u : 2u, 0u};
    unsigned values[2] = {first, second};
    for (unsigned i = 0; i < 2; i++)
    {
        unsigned from = values[i] ? writes[i] : reads[i];
        unsigned to = values[i] ? reads[i] : writes[i];
        edges[from] |= 1u << to;
    }
    if (ta_closure(edges, 4) != TA_SUCCESS)
    {
        return -1;
    }
    for (unsigned i = 0; i < 4; i++)
    {
        if (edges[i] & (1u << i))
        {
            return 0;
        }
    }
    return 1;
}

void ta_counter_init(struct ta_counter *p, unsigned value)
{
    atomic_init(&p->value, value);
}

unsigned ta_counter_add(struct ta_counter *p, unsigned value)
{
#ifdef TA_MUTANT_COUNTER
    return atomic_fetch_add_explicit(&p->value, value, memory_order_relaxed) +
           value;
#else
    return atomic_fetch_add_explicit(&p->value, value, memory_order_relaxed);
#endif
}

unsigned ta_counter_read(const struct ta_counter *p)
{
    return atomic_load_explicit(&p->value, memory_order_relaxed);
}

int ta_lockfree_admit(unsigned query, unsigned helpers, unsigned authorized)
{
    if (query > 1 || authorized > 1)
    {
        return -1;
    }
#ifdef TA_MUTANT_LOCKFREE
    (void)helpers;
    return (int)query;
#else
    return query && !helpers && authorized;
#endif
}

int ta_dependency(unsigned used, unsigned control, unsigned killed)
{
    if (used > 1 || control > 1 || killed > 1)
    {
        return -1;
    }
#ifdef TA_MUTANT_DEPENDENCY
    return used || control;
#else
    return used && !control && !killed;
#endif
}
