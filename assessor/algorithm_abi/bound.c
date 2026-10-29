#include "bound.h"

int ab_trace(const uint32_t *a, uint32_t n, uint32_t key, unsigned upper,
             struct ab_trace *out)
{
    if (!out || n > 1024 || (!a && n) || upper > 1)
        return 1;
    struct ab_trace t = {0};
    uint32_t lo = 0, hi = n;
    while (lo < hi)
    {
        uint32_t mid = lo + (hi - lo) / 2;
        struct ab_step step = {lo, hi, mid, 0, 0};
        if (a[mid] < key || (upper && a[mid] == key))
            lo = mid + 1;
        else
            hi = mid;
        step.next_lo = lo;
        step.next_hi = hi;
        t.steps[t.count++] = step;
    }
    t.index = lo;
    *out = t;
    return 0;
}

int ab_checked(const uint32_t *a, uint32_t n, uint32_t capacity, uint32_t key,
               unsigned trusted, uint32_t *out, uint32_t *validation)
{
    if (!out || !validation || trusted > 1 || n > 1024 || n > capacity ||
        (!a && n))
        return 1;
    uint32_t checks = 0;
    if (!trusted)
        for (uint32_t i = 1; i < n; ++i)
        {
            ++checks;
            if (a[i] < a[i - 1])
                return 1;
        }
    uint32_t result = algo_asm_lower(a, n, key);
    *out = result;
    *validation = checks;
    return 0;
}

int ab_records_checked(const algo_pair *a, size_t n, size_t capacity,
                       uint32_t key, size_t *out)
{
    if (!out || n > SIZE_MAX / sizeof *a || n > capacity || (!a && n))
        return 1;
    for (size_t i = 1; i < n; ++i)
        if (a[i].key < a[i - 1].key)
            return 1;
    *out = ab_records_lower(a, n, key);
    return 0;
}
