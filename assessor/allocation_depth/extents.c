#include "allocation.h"

/* Extents must be non-empty, not wrap size_t, and be sorted and disjoint. */
static int extents_valid(const struct al_extent *a, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        if (!a[i].size || a[i].offset > SIZE_MAX - a[i].size)
        {
            return 0;
        }
        if (i && a[i - 1].offset + a[i - 1].size > a[i].offset)
        {
            return 0;
        }
    }
    return 1;
}

/*
 * Index of the extent to use for `bytes`: the first sufficient one, or with
 * `best` the smallest sufficient one (the earliest on ties). Returns
 * AL_EXTENT_MAX when none is large enough.
 */
static size_t pick_extent(const struct al_extent *a, size_t count, size_t bytes,
                          unsigned best)
{
    size_t chosen = AL_EXTENT_MAX;
    for (size_t i = 0; i < count; ++i)
    {
        if (a[i].size < bytes)
        {
            continue;
        }
        if (chosen == AL_EXTENT_MAX || (best && a[i].size < a[chosen].size))
        {
            chosen = i;
        }
    }
    return chosen;
}

int al_extent_choose(struct al_extent *a, size_t *count, size_t bytes,
                     size_t minimum, unsigned best, size_t *offset,
                     size_t *charged)
{
    if (!a || !count || !bytes || !minimum || best > 1 || !offset || !charged ||
        *count > AL_EXTENT_MAX)
    {
        return AL_BAD;
    }
    if (!extents_valid(a, *count))
    {
        return AL_BAD;
    }
    size_t chosen = pick_extent(a, *count, bytes, best);
    if (chosen == AL_EXTENT_MAX)
    {
        return AL_FULL;
    }
    size_t at = a[chosen].offset;
    /* A remainder below `minimum` cannot be reused, so charge it too. */
    size_t n = a[chosen].size - bytes < minimum ? a[chosen].size : bytes;
    if (n == a[chosen].size)
    {
        for (size_t i = chosen + 1; i < *count; ++i)
        {
            a[i - 1] = a[i];
        }
        --*count;
    }
    else
    {
        a[chosen].offset += n;
        a[chosen].size -= n;
    }
    *offset = at;
    *charged = n;
    return AL_OK;
}

static void sort_by_offset(struct al_extent *a, size_t count)
{
    for (size_t i = 1; i < count; ++i)
    {
        struct al_extent moving = a[i];
        size_t j = i;
        while (j && a[j - 1].offset > moving.offset)
        {
            a[j] = a[j - 1];
            --j;
        }
        a[j] = moving;
    }
}

/*
 * Merges touching extents of an offset-sorted list in place and returns the
 * new count, or (size_t)-1 if two extents overlap.
 */
static size_t merge_touching(struct al_extent *sorted, size_t count)
{
    size_t used = 0;
    for (size_t i = 0; i < count; ++i)
    {
        if (used)
        {
            struct al_extent *previous = &sorted[used - 1];
            size_t end = previous->offset + previous->size;
            if (end > sorted[i].offset)
            {
                return (size_t)-1;
            }
            if (end == sorted[i].offset)
            {
                previous->size += sorted[i].size;
                continue;
            }
        }
        sorted[used++] = sorted[i];
    }
    return used;
}

int al_extent_coalesce(struct al_extent *a, size_t *count, size_t capacity)
{
    if (!a || !count || *count > AL_EXTENT_MAX)
    {
        return AL_BAD;
    }
    struct al_extent sorted[AL_EXTENT_MAX];
    for (size_t i = 0; i < *count; ++i)
    {
        if (!a[i].size || a[i].offset > capacity ||
            a[i].size > capacity - a[i].offset)
        {
            return AL_BAD;
        }
        sorted[i] = a[i];
    }
    sort_by_offset(sorted, *count);
    size_t used = merge_touching(sorted, *count);
    if (used == (size_t)-1)
    {
        return AL_BAD;
    }
    for (size_t i = 0; i < used; ++i)
    {
        a[i] = sorted[i];
    }
    *count = used;
    return AL_OK;
}

/* Adapters from the type-erased interface to the heap; `p` is a heap. */
static int allocate(void *p, size_t n, size_t align, struct al_handle *out)
{
    return al_heap_take(p, n, align, out);
}

static int release(void *p, struct al_handle h)
{
    return al_heap_release(p, h);
}

static int resize(void *p, struct al_handle h, size_t n, struct al_handle *out)
{
    return al_heap_resize(p, h, n, 0, SIZE_MAX, out);
}

int al_heap_api(struct al_heap *h, struct al_allocator *out)
{
    if (!h || !out)
    {
        return AL_BAD;
    }
    *out = (struct al_allocator){h, 8, allocate, release, resize};
    return AL_OK;
}

int al_api_allocate(const struct al_allocator *a, size_t n, size_t align,
                    struct al_handle *out)
{
    if (!a || !a->allocate || !a->owner || !out || !n || !align ||
        (align & (align - 1)) || align > a->max_alignment)
    {
        return AL_BAD;
    }
    return a->allocate(a->owner, n, align, out);
}
