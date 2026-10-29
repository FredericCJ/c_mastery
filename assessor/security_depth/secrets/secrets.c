#include "secrets.h"

int ss_compare(const unsigned char token[4], const unsigned char *candidate,
               size_t length, int early, struct ss_compare *out)
{
    struct ss_compare result = {1, 0};
    unsigned diff = 0;
    if (!token || !candidate || !out || length != 4 ||
        (early != 0 && early != 1))
        return -1;
    for (unsigned i = 0; i < 4; ++i)
    {
        diff |= (unsigned)(token[i] ^ candidate[i]);
        ++result.work;
#ifdef SS_MUTANT_EARLY
        if (diff)
            break;
#else
        if (early && diff)
            break;
#endif
    }
    result.equal = diff == 0;
    *out = result;
    return 0;
}

int ss_indexed(const unsigned char token[4], const unsigned char candidate[4],
               volatile const unsigned char table[256], unsigned trace[8])
{
    unsigned diff = 0;
    for (unsigned i = 0; i < 4; ++i)
    {
        trace[2 * i] = token[i];
        trace[2 * i + 1] = candidate[i];
        diff |= (unsigned)(table[token[i]] ^ table[candidate[i]]);
    }
    return diff == 0;
}

int ss_acquire(struct ss_pool *pool, const unsigned char token[4])
{
    if (!pool || !token || pool->live)
        return -1;
    for (unsigned i = 0; i < 4; ++i)
        pool->bytes[i] = token[i];
    pool->live = 1;
    return 0;
}

int ss_release(struct ss_pool *pool)
{
    if (!pool || pool->live != 1)
        return -1;
#ifndef SS_MUTANT_CLEAR
    ss_wipe(pool->bytes, sizeof pool->bytes);
#endif
    pool->live = 0;
    return 0;
}
