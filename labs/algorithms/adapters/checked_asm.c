#include "algo.h"

/* Complete boundary adapter; the learner implements the trusted .S kernels.
 * A capacity value is a caller claim, never proof that a pointer is live. */
int algo_checked_lower(const uint32_t *a, uint32_t n, uint32_t capacity,
                       uint32_t key, uint32_t *out)
{
    if (!out || n > capacity || n > 1024 || (n && !a))
    {
        return ALGO_BAD;
    }
    for (uint32_t i = 1; i < n; ++i)
    {
        if (a[i] < a[i - 1])
        {
            return ALGO_BAD;
        }
    }
    *out = algo_asm_lower(a, n, key);
    return ALGO_OK;
}

int algo_checked_insert(algo_pair *a, uint32_t n, uint32_t capacity)
{
    if (n > capacity || n > 32 || (n && !a))
    {
        return ALGO_BAD;
    }
    algo_asm_insert(a, n);
    return ALGO_OK;
}

int algo_checked_sift(uint32_t *a, uint32_t n, uint32_t capacity, uint32_t p)
{
    if (n > capacity || n > 1024 || (n && (!a || p >= n)))
    {
        return ALGO_BAD;
    }
    if (!n)
    {
        return ALGO_OK;
    }
    for (uint32_t i = 1; i < n; ++i)
    {
        uint32_t parent = (i - 1) / 2;
        if (parent != p && a[parent] < a[i])
        {
            return ALGO_BAD;
        }
        if (p && parent == p && a[(p - 1) / 2] < a[i])
        {
            return ALGO_BAD;
        }
    }
    algo_asm_sift(a, n, p);
    return ALGO_OK;
}
