#include "identity.h"

int ai_init(struct ai_pool *p, unsigned bits)
{
    if (!p || (bits != 8 && bits != 24))
    {
        return AI_INVALID;
    }
    p->maximum = (UINT32_C(1) << bits) - 1;
    for (unsigned i = 0; i < 16; i++)
    {
        p->generation[i] = 1;
        p->used[i] = 0;
        p->retired[i] = 0;
    }
    return AI_OK;
}

int ai_take(struct ai_pool *p, uint32_t *out)
{
    if (!p || !out)
    {
        return AI_INVALID;
    }
    for (unsigned i = 0; i < 16; i++)
    {
        if (!p->used[i] && !p->retired[i])
        {
            p->used[i] = 1;
            *out = (p->generation[i] << 8) | (i + 1);
            return AI_OK;
        }
    }
    return AI_FULL;
}

int ai_valid(const struct ai_pool *p, uint32_t h)
{
    unsigned i = h & 255u;
    if (!p || !i || i > 16)
    {
        return 0;
    }
    --i;
#ifdef AI_MUTANT_GENERATION
    return p->used[i] && !p->retired[i];
#else
    return p->used[i] && !p->retired[i] && (h >> 8) == p->generation[i];
#endif
}

int ai_release(struct ai_pool *p, uint32_t h)
{
    if (!ai_valid(p, h))
    {
        return AI_INVALID;
    }
    unsigned i = (h & 255u) - 1;
    p->used[i] = 0;
    if (p->generation[i] == p->maximum)
    {
#ifdef AI_MUTANT_WRAP
        p->generation[i] = 1;
#else
        p->retired[i] = 1;
#endif
    }
    else
    {
#ifndef AI_MUTANT_INCREMENT
        p->generation[i]++;
#endif
    }
    return AI_OK;
}

int ai_word_cas(_Atomic(uint32_t) *p, uint32_t *expected, uint32_t desired)
{
#ifdef AI_MUTANT_TAG
    uint32_t current = atomic_load(p);
    if ((current & 255u) == (*expected & 255u))
    {
        *expected = current;
    }
#endif
    return atomic_compare_exchange_strong(p, expected, desired);
}
