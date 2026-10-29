#include "algorithms.h"
#include "containers.h"
#include <limits.h>
_Static_assert(
    INT_MAX >= INT32_MAX && INT_MIN <= INT32_MIN,
    "T14 heap adapter requires int to represent the admitted signed32 domain");
#ifndef AD_MUTANT
#define AD_MUTANT 0
#endif
static int valid(const void *p, size_t n)
{
    return n <= 16 && (!n || p);
}

enum ad_status ad_linear(const int32_t *p, size_t n, int32_t key, size_t *out)
{
    if (!valid(p, n) || !out)
    {
        return AD_INVALID;
    }
    size_t i = 0;
    while (i < n && p[i] != key)
    {
        ++i;
    }
    *out = i;
    return AD_OK;
}

static int less(int32_t a, int32_t b, unsigned descending)
{
    return descending ? a > b : a < b;
}

static int compare(int32_t a, int32_t b, struct ad_work *w)
{
    if (w)
    {
        ++w->comparisons;
    }
    return a < b ? -1 : a > b;
}

static void store(struct ad_record *out, struct ad_record value,
                  struct ad_work *w)
{
    *out = value;
    if (w)
    {
        ++w->writes;
    }
}

static void swap(struct ad_record *a, struct ad_record *b, struct ad_work *w)
{
    if (a != b)
    {
        struct ad_record saved = *a;
        store(a, *b, w);
        store(b, saved, w);
    }
}

enum ad_status ad_bounds(const int32_t *p, size_t n, int32_t key,
                         unsigned descending, size_t *lower, size_t *upper,
                         size_t *exact)
{
    if (!valid(p, n) || descending > 1 || !lower || !upper || !exact)
    {
        return AD_INVALID;
    }
    size_t lo = 0, hi = n;
    while (lo < hi)
    {
        size_t mid = lo + (hi - lo) / 2;
        if (less(p[mid], key, descending))
        {
            lo = mid + 1;
        }
        else
        {
            hi = mid;
        }
    }
    size_t first = lo;
    hi = n;
    while (lo < hi)
    {
        size_t mid = lo + (hi - lo) / 2;
        if (!less(key, p[mid], descending))
        {
            lo = mid + 1;
        }
        else
        {
            hi = mid;
        }
    }
    *lower = first;
    *upper = lo;
    *exact = first < n && p[first] == key ? first : n;
    return AD_OK;
}

enum ad_status ad_partition(struct ad_record *p, size_t n, const int32_t *pivot,
                            unsigned stable, struct ad_record *scratch,
                            size_t capacity, size_t *cut, struct ad_work *w)
{
    if (!valid(p, n) || !pivot || !cut || stable > 1)
    {
        return AD_INVALID;
    }
    if (stable && ((n && !scratch) || capacity < n))
    {
        return AD_SPACE;
    }
    int32_t value = *pivot;
    size_t at = 0;
    if (stable)
    {
        for (size_t i = 0; i < n; ++i)
        {
            if (compare(p[i].key, value, w) < 0)
            {
                store(&scratch[at++], p[i], w);
            }
        }
        size_t split = at;
        for (size_t i = 0; i < n; ++i)
        {
            if (compare(p[i].key, value, w) >= 0)
            {
                store(&scratch[at++], p[i], w);
            }
        }
        for (size_t i = 0; i < n; ++i)
        {
            store(&p[i], scratch[i], w);
        }
        *cut = split;
    }
    else
    {
        for (size_t i = 0; i < n; ++i)
        {
            if (compare(p[i].key, value, w) < 0)
            {
                swap(&p[at++], &p[i], w);
            }
        }
        *cut = at;
    }
    return AD_OK;
}

enum ad_status ad_insertion(struct ad_record *p, size_t n, struct ad_work *w)
{
    if (!valid(p, n))
    {
        return AD_INVALID;
    }
    for (size_t i = 1; i < n; ++i)
    {
        struct ad_record value = p[i];
        size_t j = i;
        while (j && (AD_MUTANT == 1 ? compare(p[j - 1].key, value.key, w) >= 0
                                    : compare(p[j - 1].key, value.key, w) > 0))
        {
            store(&p[j], p[j - 1], w);
            --j;
        }
        store(&p[j], value, w);
    }
    return AD_OK;
}

enum ad_status ad_merge(struct ad_record *p, size_t middle, size_t n,
                        struct ad_record *scratch, size_t capacity,
                        struct ad_work *w)
{
    if (!valid(p, n) || middle > n)
    {
        return AD_INVALID;
    }
    if ((n && !scratch) || capacity < n)
    {
        return AD_SPACE;
    }
    size_t i = 0, j = middle, used = 0;
    while (i < middle && j < n)
    {
        if (compare(p[i].key, p[j].key, w) <= 0)
        {
            store(&scratch[used++], p[i++], w);
        }
        else
        {
            store(&scratch[used++], p[j++], w);
        }
    }
    while (i < middle)
    {
        store(&scratch[used++], p[i++], w);
    }
    while (j < n)
    {
        store(&scratch[used++], p[j++], w);
    }
    for (i = 0; i < n; ++i)
    {
        store(&p[i], scratch[i], w);
    }
    return AD_OK;
}

static void merge_sort(struct ad_record *p, size_t n, struct ad_record *scratch,
                       struct ad_work *w)
{
    if (n < 2)
    {
        return;
    }
    size_t mid = n / 2;
    merge_sort(p, mid, scratch, w);
    merge_sort(p + mid, n - mid, scratch, w);
    ad_merge(p, mid, n, scratch, n, w);
}

enum ad_status ad_merge_sort(struct ad_record *p, size_t n,
                             struct ad_record *scratch, size_t capacity,
                             struct ad_work *w)
{
    if (!valid(p, n))
    {
        return AD_INVALID;
    }
    if ((n && !scratch) || capacity < n)
    {
        return AD_SPACE;
    }
    if (n)
    {
        merge_sort(p, n, scratch, w);
    }
    return AD_OK;
}

enum ad_status ad_heap_sort(struct ad_record *p, size_t n, struct ad_work *w)
{
    if (!valid(p, n))
    {
        return AD_INVALID;
    }
    struct cd_heap heap;
    int keys[16];
    unsigned labels[16];
    for (size_t i = 0; i < n; ++i)
    {
        keys[i] = p[i].key;
        labels[i] = p[i].label;
    }
    if (cd_heap_build(&heap, n ? keys : NULL, n, 1) != CD_OK)
    {
        return AD_FAILED;
    }
    for (size_t i = n; i > 0; --i)
    {
        int key;
        unsigned identity;
        if (cd_heap_take(&heap, &key, &identity) != CD_OK)
        {
            return AD_FAILED;
        }
        store(&p[i - 1], (struct ad_record){key, labels[identity]}, w);
    }
    if (w)
    {
        w->comparisons += heap.comparisons;
    }
    return AD_OK;
}

enum ad_status ad_three_way(struct ad_record *p, size_t n, const int32_t *pivot,
                            size_t *lower, size_t *upper, struct ad_work *w)
{
    if (!valid(p, n) || !pivot || !lower || !upper)
    {
        return AD_INVALID;
    }
    int32_t value = *pivot;
    size_t lt = 0, at = 0, gt = n;
    while (at < gt)
    {
        int relation = compare(p[at].key, AD_MUTANT == 2 ? *pivot : value, w);
        if (relation < 0)
        {
            swap(&p[lt++], &p[at++], w);
        }
        else if (relation > 0)
        {
            swap(&p[at], &p[--gt], w);
        }
        else
        {
            ++at;
        }
    }
    *lower = lt;
    *upper = gt;
    return AD_OK;
}

static void quick(struct ad_record *p, size_t n, unsigned budget,
                  unsigned depth, struct ad_work *w)
{
    if (w && w->max_depth < depth)
    {
        w->max_depth = depth;
    }
    while (n > 1)
    {
        if (!budget)
        {
            if (w)
            {
                ++w->fallbacks;
            }
            ad_heap_sort(p, n, w);
            return;
        }
        --budget;
        size_t lo, hi;
        ad_three_way(p, n, &p[0].key, &lo, &hi, w);
        size_t right = n - hi;
        if (lo < right)
        {
            if (lo > 1)
            {
                quick(p, lo, budget, depth + 1, w);
            }
            p += hi;
            n = right;
        }
        else
        {
            if (right > 1)
            {
                quick(p + hi, right, budget, depth + 1, w);
            }
            n = lo;
        }
    }
}

enum ad_status ad_quick_sort(struct ad_record *p, size_t n, unsigned budget,
                             unsigned explicit_stack, struct ad_work *w)
{
    if (!valid(p, n) || explicit_stack > 1)
    {
        return AD_INVALID;
    }
    if (!explicit_stack)
    {
        if (n)
        {
            quick(p, n, budget, 1, w);
        }
        return AD_OK;
    } /* Each nested deferral continues a side at most half its parent size, hence depth<=floor(log2(16))+1. */

    struct ad_frame
    {
        size_t start, count;
        unsigned budget;
    } frames[5];

    size_t used = 0, start = 0, count = n;
    for (;;)
    {
        while (count > 1)
        {
            if (!budget)
            {
                if (w)
                {
                    ++w->fallbacks;
                }
                ad_heap_sort(p + start, count, w);
                count = 0;
                break;
            }
            --budget;
            size_t lo, hi;
            ad_three_way(p + start, count, &p[start].key, &lo, &hi, w);
            size_t right =
                count -
                hi; /* Defer the larger partition; continue the smaller one. */
            if (lo < right)
            {
                if (right > 1)
                {
                    frames[used++] =
                        (struct ad_frame){start + hi, right, budget};
                }
                count = lo;
            }
            else
            {
                if (lo > 1)
                {
                    frames[used++] = (struct ad_frame){start, lo, budget};
                }
                start += hi;
                count = right;
            }
            if (w && w->max_depth < used)
            {
                w->max_depth = (unsigned)used;
            }
        }
        if (!used)
        {
            break;
        }
        struct ad_frame f = frames[--used];
        start = f.start;
        count = f.count;
        budget = f.budget;
    }
    return AD_OK;
}

enum ad_status ad_select(struct ad_record *p, size_t n, size_t rank,
                         unsigned one_based, int32_t *out, size_t *lower,
                         size_t *upper, struct ad_work *w)
{
    if (!valid(p, n) || !out || !lower || !upper || one_based > 1)
    {
        return AD_INVALID;
    }
    if (one_based)
    {
        if (!rank)
        {
            return AD_INVALID;
        }
        --rank;
    }
    if (rank >= n)
    {
        return AD_INVALID;
    }
    size_t start = 0, count = n;
    for (;;)
    {
        size_t lo, hi;
        ad_three_way(p + start, count, &p[start].key, &lo, &hi, w);
        if (rank < start + lo)
        {
            count = lo;
        }
        else if (rank >= start + hi)
        {
            start += hi;
            count -= hi;
        }
        else
        {
            *out = p[rank].key;
            *lower = start + lo;
            *upper = start + hi;
            return AD_OK;
        }
    }
}

enum ad_status ad_counting(struct ad_record *p, size_t n,
                           struct ad_record *scratch, size_t capacity,
                           struct ad_work *w)
{
    if (!valid(p, n))
    {
        return AD_INVALID;
    }
    if ((n && !scratch) || capacity < n)
    {
        return AD_SPACE;
    }
    size_t counts[10] = {0};
    for (size_t i = 0; i < n; ++i)
    {
        if (p[i].key < -2 || p[i].key > 7)
        {
            return AD_INVALID;
        }
        ++counts[(unsigned)(p[i].key + 2)];
    }
    size_t total = 0;
    for (unsigned i = 0; i < 10; ++i)
    {
        size_t count = counts[i];
        counts[i] = total;
        total += count;
    }
    for (size_t i = 0; i < n; ++i)
    {
        store(&scratch[counts[(unsigned)(p[i].key + 2)]++], p[i], w);
    }
    for (size_t i = 0; i < n; ++i)
    {
        store(&p[i], scratch[i], w);
    }
    return AD_OK;
}

static uint32_t ordered_key(int32_t value)
{ /* Numeric conversion modulo2^32; independent of host byte layout. */
    return (uint32_t)value - UINT32_C(0x80000000);
}

enum ad_status ad_radix(struct ad_record *p, size_t n, unsigned digit_bits,
                        struct ad_record *scratch, size_t capacity,
                        struct ad_work *w)
{
    if (!valid(p, n) || (digit_bits != 4 && digit_bits != 8))
    {
        return AD_INVALID;
    }
    if ((n && !scratch) || capacity < n)
    {
        return AD_SPACE;
    }
    unsigned radix = 1u << digit_bits, mask = radix - 1;
    for (unsigned shift = 0; shift < 32; shift += digit_bits)
    {
        size_t counts[256] = {0};
        for (size_t i = 0; i < n; ++i)
        {
            ++counts[(ordered_key(p[i].key) >> shift) & mask];
        }
        size_t total = 0;
        for (unsigned i = 0; i < radix; ++i)
        {
            size_t count = counts[i];
            counts[i] = total;
            total += count;
        }
        for (size_t i = 0; i < n; ++i)
        {
            store(&scratch[counts[(ordered_key(p[i].key) >> shift) & mask]++],
                  p[i], w);
        }
        for (size_t i = 0; i < n; ++i)
        {
            store(&p[i], scratch[i], w);
        }
    }
    return AD_OK;
}
