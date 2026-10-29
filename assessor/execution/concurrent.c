#include "cmx.h"
#define NO_NODE UINT32_MAX

int cmx_spsc_init(cmx_spsc *q, unsigned capacity)
{
    if (!capacity || capacity > 16 || (capacity & (capacity - 1)))
    {
        return 0;
    }
    atomic_init(&q->head, 0);
    atomic_init(&q->tail, 0);
    q->capacity = capacity;
    return atomic_is_lock_free(&q->head) && atomic_is_lock_free(&q->tail);
}

enum cmx_result cmx_spsc_push(cmx_spsc *q, uint32_t value)
{
    unsigned h = atomic_load_explicit(&q->head, memory_order_relaxed),
             t = atomic_load_explicit(&q->tail, memory_order_acquire);
    if ((unsigned)(h - t) >= q->capacity)
    {
        return CMX_FULL;
    }
    q->data[h & (q->capacity - 1)] = value;
    atomic_store_explicit(&q->head, h + 1, memory_order_release);
    return CMX_OK;
}

enum cmx_result cmx_spsc_pop(cmx_spsc *q, uint32_t *value)
{
    unsigned t = atomic_load_explicit(&q->tail, memory_order_relaxed),
             h = atomic_load_explicit(&q->head, memory_order_acquire);
    if (t == h)
    {
        return CMX_RETRY;
    }
    *value = q->data[t & (q->capacity - 1)];
    atomic_store_explicit(&q->tail, t + 1, memory_order_release);
    return CMX_OK;
}

void cmx_hp_init(cmx_hp_queue *q)
{
    atomic_init(&q->head, 0);
    atomic_init(&q->tail, 0);
    for (unsigned p = 0; p < CMX_HP_THREADS; p++)
    {
        for (unsigned h = 0; h < 2; h++)
        {
            atomic_init(&q->hazard[p][h], NO_NODE);
        }
    }
    for (unsigned i = 0; i < CMX_HP_NODES; i++)
    {
        atomic_init(&q->node[i].next, NO_NODE);
        atomic_init(&q->node[i].state, i ? 0u : 1u);
        q->node[i].value = 0;
    }
}

static unsigned protect(cmx_hp_queue *q, unsigned p, unsigned h,
                        atomic_uint *location)
{
    unsigned n;
    do
    {
        n = atomic_load(location);
        atomic_store(&q->hazard[p][h], n);
    } while (n != atomic_load(location));
    return n;
}

static void clear(cmx_hp_queue *q, unsigned p)
{
    atomic_store(&q->hazard[p][0], NO_NODE);
    atomic_store(&q->hazard[p][1], NO_NODE);
}

unsigned cmx_hp_collect(cmx_hp_queue *q)
{
    unsigned count = 0;
    for (unsigned n = 0; n < CMX_HP_NODES; n++)
    {
        unsigned old = atomic_load(&q->node[n].state);
        /* The retirement generation is part of the CAS. Otherwise a stalled
         * collector could free a later incarnation after another collector reused
         * and retired the same slot. Exhaust the slot rather than wrap its tag. */
        if ((old & 3u) != 2u || old >= UINT32_MAX - 3u)
        {
            continue;
        }
        unsigned protected = 0;
        for (unsigned p = 0; p < CMX_HP_THREADS; p++)
        {
            for (unsigned h = 0; h < 2; h++)
            {
                if (atomic_load(&q->hazard[p][h]) == n)
                {
                  protected
                }
            }
        }
        = 1;
        if (!protected && atomic_compare_exchange_strong(
                              &q->node[n].state, &old, (old & ~3u) + 4u))
        {
            count++;
        }
    }
    return count;
}

enum cmx_result cmx_hp_push(cmx_hp_queue *q, unsigned p, uint32_t value)
{
    if (p >= CMX_HP_THREADS)
    {
        return CMX_INVALID;
    }
    unsigned n = NO_NODE;
    for (unsigned i = 0; i < CMX_HP_NODES; i++)
    {
        unsigned old = atomic_load(&q->node[i].state);
        if ((old & 3u) == 0 &&
            atomic_compare_exchange_strong(&q->node[i].state, &old, old | 1u))
        {
            n = i;
            break;
        }
    }
    if (n == NO_NODE)
    {
        return CMX_FULL;
    }
    q->node[n].value = value;
    atomic_store(&q->node[n].next, NO_NODE);
    for (;;)
    {
        unsigned tail = protect(q, p, 0, &q->tail),
                 next = atomic_load(&q->node[tail].next);
        if (tail != atomic_load(&q->tail))
        {
            continue;
        }
        if (next == NO_NODE)
        {
            unsigned expected = NO_NODE;
            if (atomic_compare_exchange_weak(&q->node[tail].next, &expected, n))
            {
                atomic_compare_exchange_strong(&q->tail, &tail, n);
                clear(q, p);
                return CMX_OK;
            }
        }
        else
        {
            atomic_compare_exchange_weak(&q->tail, &tail, next);
        }
    }
}

enum cmx_result cmx_hp_pop(cmx_hp_queue *q, unsigned p, uint32_t *value)
{
    if (p >= CMX_HP_THREADS || !value)
    {
        return CMX_INVALID;
    }
    for (;;)
    {
        unsigned head = protect(q, p, 0, &q->head),
                 tail = atomic_load(&q->tail),
                 next = protect(q, p, 1, &q->node[head].next);
        if (head != atomic_load(&q->head))
        {
            continue;
        }
        if (next == NO_NODE)
        {
            clear(q, p);
            return CMX_RETRY;
        }
        if (head == tail)
        {
            atomic_compare_exchange_weak(&q->tail, &tail, next);
            continue;
        }
        uint32_t item = q->node[next].value;
        unsigned old = head;
        if (atomic_compare_exchange_weak(&q->head, &old, next))
        {
            *value = item;
            atomic_fetch_add(&q->node[head].state,
                             1u); /* LIVE -> RETIRED, same tag */
            clear(q, p);
            return CMX_OK;
        }
    }
}

void cmx_epoch_init(cmx_epoch *e)
{
    *e = (cmx_epoch){0};
}

enum cmx_result cmx_epoch_enter(cmx_epoch *e, unsigned p)
{
    if (p >= CMX_HP_THREADS || e->active[p])
    {
        return CMX_INVALID;
    }
    e->seen[p] = e->epoch;
    e->active[p] = 1;
    return CMX_OK;
}

void cmx_epoch_leave(cmx_epoch *e, unsigned p)
{
    if (p < CMX_HP_THREADS)
    {
        e->active[p] = 0;
    }
}

int cmx_epoch_advance(cmx_epoch *e)
{
    if (e->epoch == UINT32_MAX)
    {
        return 0;
    }
    for (unsigned p = 0; p < CMX_HP_THREADS; p++)
    {
        if (e->active[p] && e->seen[p] != e->epoch)
        {
            return 0;
        }
    }
    e->epoch++;
    return 1;
}
