#include "metrics.h"
#include <stdbool.h>

enum
{
    MAX_SEPARATED_ELEMENTS = 4,
    MAX_ACCUMULATE_ITEMS = 65536,
    MAX_STEP = 4,
    MAX_MODE = 2,
    MAX_BATCH = 65536,
    MAX_INTENSITY = 64
};

/*
 * 1 when the first `count` elements, each `extent` bytes wide and `stride`
 * bytes apart starting at address `base`, occupy distinct cache lines of
 * `line` bytes (a power of two); 0 when two share a line; -1 for arguments
 * outside the model or address arithmetic that would wrap.
 */
int tp_separated(uintptr_t base, size_t stride, size_t count, size_t extent,
                 size_t line)
{
    if (!line || (line & (line - 1)) || !extent || extent > line || !stride ||
        count > MAX_SEPARATED_ELEMENTS)
    {
        return -1;
    }
    uintptr_t previous = 0;
    for (size_t i = 0; i < count; i++)
    {
        if (i && stride > UINTPTR_MAX / i)
        {
            return -1;
        }
        uintptr_t offset = (uintptr_t)i * stride;
        if (offset > UINTPTR_MAX - base ||
            extent - 1 > UINTPTR_MAX - (base + offset))
        {
            return -1;
        }
        uintptr_t first = (base + offset) / line;
        uintptr_t last = (base + offset + extent - 1) / line;
#ifdef TP_MUTANT_LAYOUT
        (void)first;
        (void)previous;
#else
        if (i && first <= previous)
        {
            return 0;
        }
#endif
        previous = last;
    }
    return 1;
}

uint32_t tp_mix(uint32_t value, unsigned intensity)
{
    for (unsigned i = 0; i < intensity; i++)
    {
        value =
            (value * UINT32_C(1664525) + UINT32_C(1013904223)) ^ (value >> 7);
    }
#ifdef TP_MUTANT_COST
    value++;
#endif
    return value;
}

/*
 * Sums data[begin], data[begin + step], ... and folds the same items into
 * `shared` according to mode: 0 never touches it, 1 publishes a batch every
 * `batch` items (plus a final partial batch), 2 publishes every item.
 * Returns TP_INVALID without touching *out or *shared for bad arguments.
 * The shared updates are relaxed: only the final total matters, not the
 * order in which other threads observe the partial ones.
 */
int tp_accumulate(const unsigned *data, size_t n, unsigned begin, unsigned step,
                  unsigned mode, unsigned batch, unsigned intensity,
                  _Atomic(uint64_t) *shared, struct tp_work *out)
{
    bool bad_data = (n && !data) || n > MAX_ACCUMULATE_ITEMS;
    bool bad_stride = !step || step > MAX_STEP || begin >= step;
    bool bad_policy = mode > MAX_MODE || !batch || batch > MAX_BATCH;
    if (bad_data || bad_stride || bad_policy || !shared || !out ||
        intensity > MAX_INTENSITY)
    {
        return TP_INVALID;
    }
    struct tp_work r = {0};
    uint64_t pending = 0; /* unpublished sum of the batch (mode 1) */
    unsigned used = 0; /* items in that batch */
    for (size_t i = begin; i < n; i += step)
    {
        r.sum += data[i];
        r.checksum += tp_mix(data[i], intensity);
        r.items++;
        if (mode == 1)
        {
            pending += data[i];
            used++;
            if (used == batch)
            {
                atomic_fetch_add_explicit(shared, pending,
                                          memory_order_relaxed);
                r.shared_updates++;
                pending = 0;
                used = 0;
            }
        }
        else if (mode == 2)
        {
            atomic_fetch_add_explicit(shared, data[i], memory_order_relaxed);
            r.shared_updates++;
        }
    }
    if (mode == 1 && used)
    {
        atomic_fetch_add_explicit(shared, pending, memory_order_relaxed);
        r.shared_updates++;
    }
#ifdef TP_MUTANT_AGGREGATION
    r.sum++;
#endif
    *out = r;
    return TP_OK;
}

/*
 * Mutex-guarded running total. value and items change together under the
 * lock, so a reader never sees one without the other. tp_total_add returns
 * TP_OVERFLOW (and changes nothing) instead of wrapping.
 */
int tp_total_init(struct tp_total *t)
{
    int e = pthread_mutex_init(&t->lock, NULL);
    if (e)
    {
        return e;
    }
    t->value = 0;
    t->items = 0;
    return 0;
}

int tp_total_add(struct tp_total *t, uint64_t value)
{
    int e = pthread_mutex_lock(&t->lock);
    if (e)
    {
        return e;
    }
    int status = TP_OK;
    if (t->items == UINT64_MAX || value > UINT64_MAX - t->value)
    {
        status = TP_OVERFLOW;
    }
    else
    {
        t->value += value;
#ifndef TP_MUTANT_FRESHNESS
        t->items++;
#endif
    }
    e = pthread_mutex_unlock(&t->lock);
    return e ? e : status;
}

int tp_total_read(struct tp_total *t, uint64_t *value, uint64_t *items)
{
    int e = pthread_mutex_lock(&t->lock);
    if (e)
    {
        return e;
    }
    *value = t->value;
    *items = t->items;
    e = pthread_mutex_unlock(&t->lock);
    return e;
}

int tp_total_destroy(struct tp_total *t)
{
    return pthread_mutex_destroy(&t->lock);
}
