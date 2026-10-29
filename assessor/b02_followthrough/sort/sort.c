#include "sort.h"
#ifndef BF_MUTANT
#define BF_MUTANT 0
#endif
int bf_sort_width(size_t n, size_t width, size_t *out)
{
    if (!out || n < 2 || !width || width >= n)
        return B2_INVALID;
    *out = width > n - width ? n : width + width;
    return B2_OK;
}

int bf_sort_bottom(struct b2_item *a, size_t n, struct b2_item *s,
                   size_t capacity, struct bf_work *out)
{
    if (!out || (n && !a))
        return B2_INVALID;
    if (n > 64)
        return B2_RANGE;
    if (n > 1 && (!s || capacity < n))
    {
        if (BF_MUTANT == 3)
            a[0].key = 0;
        return B2_FULL;
    }
    struct bf_work work = {0, 0, 0};
    for (size_t width = 1; width < n;)
    {
        for (size_t start = 0; start < n;)
        {
            size_t left = n - start < width ? n - start : width;
            size_t middle = start + left;
            size_t right = n - middle < width ? n - middle : width;
            size_t end = middle + right, i = start, j = middle, k = start;
            while (i < middle && j < end)
            {
                ++work.comparisons;
                int take_left =
                    BF_MUTANT == 2 ? a[i].key < a[j].key : a[i].key <= a[j].key;
                s[k++] = take_left ? a[i++] : a[j++];
                ++work.writes;
            }
            while (i < middle)
            {
                s[k++] = a[i++];
                ++work.writes;
            }
            while (j < end)
            {
                s[k++] = a[j++];
                ++work.writes;
            }
            start = end;
        }
        for (size_t i = 0; i < n; ++i)
        {
            a[i] = s[i];
            ++work.writes;
        }
        size_t next;
        bf_sort_width(n, width, &next);
        width = next;
    }
    work.payload_writes = work.writes;
    *out = work;
    return B2_OK;
}

int bf_sort_indirect(const struct bf_record *a, size_t n, size_t *indices,
                     size_t capacity, size_t *scratch, size_t scratch_capacity,
                     struct bf_work *out)
{
    if (!out || (n && (!a || !indices)))
        return B2_INVALID;
    if (n > 64)
        return B2_RANGE;
    if (capacity < n || (n > 1 && (!scratch || scratch_capacity < n)))
        return B2_FULL;
    struct bf_work work = {0, 0, 0};
    for (size_t i = 0; i < n; ++i)
    {
        indices[i] = BF_MUTANT == 4 ? 0 : i;
        ++work.writes;
    }
    for (size_t width = 1; width < n;)
    {
        for (size_t start = 0; start < n;)
        {
            size_t left = n - start < width ? n - start : width,
                   middle = start + left;
            size_t right = n - middle < width ? n - middle : width,
                   end = middle + right;
            size_t i = start, j = middle, k = start;
            while (i < middle && j < end)
            {
                ++work.comparisons;
                scratch[k++] = a[indices[i]].key <= a[indices[j]].key
                                   ? indices[i++]
                                   : indices[j++];
                ++work.writes;
            }
            while (i < middle)
            {
                scratch[k++] = indices[i++];
                ++work.writes;
            }
            while (j < end)
            {
                scratch[k++] = indices[j++];
                ++work.writes;
            }
            start = end;
        }
        for (size_t i = 0; i < n; ++i)
        {
            indices[i] = scratch[i];
            ++work.writes;
        }
        size_t next;
        bf_sort_width(n, width, &next);
        width = next;
    }
    *out = work;
    return B2_OK;
}
