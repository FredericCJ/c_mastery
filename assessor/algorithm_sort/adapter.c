#include "sort.h"
#include "algorithms.h"
#include <limits.h>
_Static_assert(UINT_MAX >= UINT32_MAX,
               "prior label preserves all unsigned32 tags");

static int32_t ordered(uint32_t key)
{
    return key <= INT32_MAX ? INT32_MIN + (int32_t)key
                            : (int32_t)(key - UINT32_C(2147483648));
}

static uint32_t original(int32_t key)
{
    return key < 0 ? (uint32_t)(key - INT32_MIN)
                   : (uint32_t)key + UINT32_C(2147483648);
}

int as_prior(const algo_pair *a, uint32_t n, unsigned method, algo_pair *out,
             struct as_prior_cost *cost)
{
    if (n > 16 || method > 2 || (!a && n) || (!out && n) || !cost)
        return 1;
    struct ad_record records[16], scratch[16];
    struct ad_work work = {0};
    for (uint32_t i = 0; i < n; ++i)
        records[i] = (struct ad_record){ordered(a[i].key), a[i].tag};
    enum ad_status status = method == 0 ? ad_insertion(records, n, &work)
                            : method == 1
                                ? ad_merge_sort(records, n, scratch, 16, &work)
                                : ad_heap_sort(records, n, &work);
    if (status != AD_OK)
        return 2;
    for (uint32_t i = 0; i < n; ++i)
        out[i] = (algo_pair){original(records[i].key), records[i].label};
    *cost = (struct as_prior_cost){work.comparisons, work.writes};
    return 0;
}

int as_prior_merge(const algo_pair *a, uint32_t n, uint32_t middle,
                   algo_pair *out, struct as_prior_cost *cost)
{
    if (n > 16 || middle > n || (!a && n) || (!out && n) || !cost)
        return 1;
    for (uint32_t i = 1; i < n; ++i)
        if (i != middle && a[i].key < a[i - 1].key)
            return 1;
    struct ad_record records[16], scratch[16];
    struct ad_work work = {0};
    for (uint32_t i = 0; i < n; ++i)
        records[i] = (struct ad_record){ordered(a[i].key), a[i].tag};
    if (ad_merge(records, middle, n, scratch, 16, &work) != AD_OK)
        return 2;
    for (uint32_t i = 0; i < n; ++i)
        out[i] = (algo_pair){original(records[i].key), records[i].label};
    *cost = (struct as_prior_cost){work.comparisons, work.writes};
    return 0;
}
