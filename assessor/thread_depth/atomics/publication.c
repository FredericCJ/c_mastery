#include "atomics.h"
#include <limits.h>

enum
{
    MAX_SNAPSHOT_ATTEMPTS = 16,
    MAX_SLOT_READERS = 2,
    MAX_CLOSURE_EVENTS = 8
};

/*
 * Sequence-lock snapshot: a single writer makes `sequence` odd while it
 * updates a and b, and even afterwards. The atomic operations below keep
 * their default (sequentially consistent) order unchanged.
 */
void ta_snapshot_init(struct ta_snapshot *s)
{
    atomic_init(&s->sequence, 0);
    atomic_init(&s->a, 50);
    atomic_init(&s->b, 50);
}

int ta_snapshot_write(struct ta_snapshot *s, int a)
{
    unsigned n = atomic_load(&s->sequence);
    if (a < 0 || a > 100)
    {
        return TA_INVALID;
    }
    if (n > UINT_MAX - 2u)
    {
        return TA_LIMIT;
    }
    atomic_store(&s->sequence, n + 1);
    atomic_store(&s->a, a);
    atomic_store(&s->b, 100 - a);
    atomic_store(&s->sequence, n + 2);
    return TA_SUCCESS;
}

int ta_snapshot_try(struct ta_snapshot *s, unsigned attempts, int *out)
{
    if (!attempts || attempts > MAX_SNAPSHOT_ATTEMPTS || !out)
    {
        return TA_INVALID;
    }
    for (unsigned i = 0; i < attempts; i++)
    {
        unsigned before = atomic_load(&s->sequence);
        if ((before & 1u)
#ifdef TA_MUTANT_SNAPSHOT
            && 0
#endif
        )
            continue;
        int a = atomic_load(&s->a), b = atomic_load(&s->b);
        unsigned after = atomic_load(&s->sequence);
        if (before == after)
        {
            *out = a + b;
            return TA_SUCCESS;
        }
    }
    return TA_LIMIT;
}

/*
 * Single-producer slot handed to 1 or 2 enrolled readers. `pending` holds one
 * bit per reader that has not yet taken the current payload; the producer may
 * publish again only when it is zero.
 */
int ta_slot_init(struct ta_slot *s, unsigned readers)
{
    if (readers < 1 || readers > MAX_SLOT_READERS)
    {
        return TA_INVALID;
    }
    atomic_init(&s->pending, 0);
    s->readers = readers;
    s->payload = 0;
    return TA_SUCCESS;
}

int ta_slot_publish(struct ta_slot *s, int value)
{
    if (atomic_load_explicit(&s->pending, memory_order_acquire) != 0
#ifdef TA_MUTANT_HANDOFF
        && 0
#endif
    )
        return TA_LIMIT;
    s->payload = value;
    atomic_store_explicit(&s->pending, (1u << s->readers) - 1u,
                          memory_order_release);
    return TA_SUCCESS;
}

int ta_slot_read(struct ta_slot *s, unsigned reader, int *out)
{
    if (reader >= s->readers || !out)
    {
        return TA_INVALID;
    }
    unsigned mask = 1u << reader;
    if (!(atomic_load_explicit(&s->pending, memory_order_acquire) & mask))
    {
        return TA_LIMIT;
    }
    int value = s->payload;
    atomic_fetch_and_explicit(&s->pending, ~mask, memory_order_acq_rel);
    *out = value;
    return TA_SUCCESS;
}

/* Two accesses race when they touch the same location, at least one
 * writes, the accesses are not both atomic and neither happens before the
 * other. -1 for non-boolean input. */
int ta_data_race(unsigned same, unsigned a, unsigned b, unsigned atomic,
                 unsigned ordered)
{
    if (same > 1 || a > 1 || b > 1 || atomic > 1 || ordered > 1)
    {
        return -1;
    }
    return same && (a || b) && !atomic && !ordered
#ifdef TA_MUTANT_CONFLICT
           && 0
#endif
        ;
}

/*
 * Warshall transitive closure on at most MAX_CLOSURE_EVENTS events. Returns
 * TA_INVALID, leaving edges untouched, for a null graph, n == 0, an oversized
 * graph or an edge to a nonexistent event.
 */
int ta_closure(unsigned *edges, size_t n)
{
    if (!edges || !n || n > MAX_CLOSURE_EVENTS)
    {
        return TA_INVALID;
    }
    unsigned allowed = (1u << n) - 1u;
    for (size_t i = 0; i < n; i++)
    {
        if (edges[i] & ~allowed)
        {
            return TA_INVALID;
        }
    }
    for (size_t k = 0; k < n
#ifdef TA_MUTANT_GRAPH
                       && k == 0
#endif
         ;
         k++)
        for (size_t i = 0; i < n; i++)
        {
            if (edges[i] & (1u << k))
            {
                edges[i] |= edges[k];
            }
        }
    return TA_SUCCESS;
}
