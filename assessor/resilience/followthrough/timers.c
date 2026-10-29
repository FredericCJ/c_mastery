#include "timers.h"
#ifndef RZT_NO_WORK
#define WORK_ADD(t, field, value) ((t)->work.field += (value))
#else
#define WORK_ADD(t, field, value) ((void)0)
#endif

static bool kind_valid(enum rzt_kind kind)
{
    return kind >= RZT_LIST && kind <= RZT_WHEEL;
}

static bool less(struct rzt_timers *t, int a, int b)
{
    WORK_ADD(t, comparisons, 1);
    return t->nodes[a].due < t->nodes[b].due ||
           (t->nodes[a].due == t->nodes[b].due &&
            t->nodes[a].sequence < t->nodes[b].sequence);
}

static void link_before(struct rzt_timers *t, int slot, int bucket, int next)
{
    struct rzt_node *n = &t->nodes[slot];
    int previous = next < 0 ? -1 : t->nodes[next].previous;
    if (next < 0)
    {
        previous = t->heads[bucket];
        while (previous >= 0 && t->nodes[previous].next >= 0)
        {
            previous = t->nodes[previous].next;
        }
    }
    n->bucket = bucket;
    n->previous = previous;
    n->next = next;
    if (previous >= 0)
    {
        t->nodes[previous].next = slot;
    }
    else
    {
        t->heads[bucket] = slot;
    }
    if (next >= 0)
    {
        t->nodes[next].previous = slot;
    }
    WORK_ADD(t, link_writes, 4 + (next >= 0));
}

static void unlink_node(struct rzt_timers *t, int slot)
{
    struct rzt_node *n = &t->nodes[slot];
    if (n->previous >= 0)
    {
        t->nodes[n->previous].next = n->next;
    }
    else
    {
        t->heads[n->bucket] = n->next;
    }
    int had_next = n->next >= 0;
    if (had_next)
    {
        t->nodes[n->next].previous = n->previous;
    }
    n->previous = n->next = -1;
    n->bucket = -1;
    WORK_ADD(t, link_writes, 4 + (unsigned)had_next);
}

static void heap_swap(struct rzt_timers *t, unsigned a, unsigned b)
{
    int value = t->heap[a];
    t->heap[a] = t->heap[b];
    t->heap[b] = value;
    t->nodes[t->heap[a]].position = (int)a;
    t->nodes[t->heap[b]].position = (int)b;
    WORK_ADD(t, link_writes, 4);
}

static void heap_up(struct rzt_timers *t, unsigned at)
{
    while (at && less(t, t->heap[at], t->heap[(at - 1) / 2]))
    {
        heap_swap(t, at, (at - 1) / 2);
        at = (at - 1) / 2;
    }
}

static void heap_down(struct rzt_timers *t, unsigned at)
{
    for (;;)
    {
        unsigned child = at * 2 + 1;
        if (child >= t->heap_count)
        {
            return;
        }
        if (child + 1 < t->heap_count &&
            less(t, t->heap[child + 1], t->heap[child]))
        {
            ++child;
        }
        if (!less(t, t->heap[child], t->heap[at]))
        {
            return;
        }
        heap_swap(t, at, child);
        at = child;
    }
}

static void heap_remove(struct rzt_timers *t, int slot)
{
    unsigned at = (unsigned)t->nodes[slot].position;
    --t->heap_count;
    if (at < t->heap_count)
    {
        t->heap[at] = t->heap[t->heap_count];
        t->nodes[t->heap[at]].position = (int)at;
        WORK_ADD(t, link_writes, 2);
        if (at && less(t, t->heap[at], t->heap[(at - 1) / 2]))
        {
            heap_up(t, at);
        }
        else
        {
            heap_down(t, at);
        }
    }
    t->nodes[slot].position = -1;
    WORK_ADD(t, link_writes, 1);
}

static int wheel_bucket(const struct rzt_timers *t, uint64_t due)
{
    return due - t->now < 8 ? (int)(due & 7u) : 8 + (int)((due / 8) & 7u);
}

int rzt_init(struct rzt_timers *t, enum rzt_kind kind, uint64_t now)
{
    if (!t || !kind_valid(kind))
    {
        return RZ_INVALID;
    }
    *t = (struct rzt_timers){0};
    t->kind = kind;
    t->now = t->observed = now;
    for (unsigned i = 0; i < 16; ++i)
    {
        t->heads[i] = -1;
        t->nodes[i].previous = t->nodes[i].next = t->nodes[i].position =
            t->nodes[i].bucket = -1;
    }
    return RZ_OK;
}

int rzt_schedule(struct rzt_timers *t, uint64_t observed, uint64_t delay,
                 struct rzt_id *out)
{
    if (!t || !out || !kind_valid(t->kind))
    {
        return RZ_INVALID;
    }
    if (observed < t->observed || observed < t->now || delay > 63 ||
        delay > UINT64_MAX - observed)
    {
        return RZ_RANGE;
    }
    if (!delay)
    {
        return RZ_EXPIRED;
    }
    if (t->sequence == UINT64_MAX)
    {
        return RZ_FULL;
    }
    int slot = -1;
    for (unsigned i = 0; i < 16; ++i)
    {
        if (!t->nodes[i].active && t->nodes[i].generation < UINT32_MAX)
        {
            slot = (int)i;
            break;
        }
    }
    if (slot < 0)
    {
        return RZ_FULL;
    }
    struct rzt_node *n = &t->nodes[slot];
    n->due = observed + delay;
    n->sequence = t->sequence++;
    ++n->generation;
    n->active = true;
    t->observed = observed;
    ++t->count;
    if (t->kind == RZT_HEAP)
    {
        unsigned at = t->heap_count++;
        t->heap[at] = slot;
        n->position = (int)at;
        WORK_ADD(t, link_writes, 2);
        heap_up(t, at);
    }
    else if (t->kind == RZT_LIST)
    {
        int next = t->heads[0];
        while (next >= 0 && !less(t, slot, next))
        {
            next = t->nodes[next].next;
        }
        link_before(t, slot, 0, next);
    }
    else
    {
        int bucket = wheel_bucket(t, n->due);
        link_before(t, slot, bucket, t->heads[bucket]);
    }
    *out = (struct rzt_id){t, (unsigned)slot, n->generation};
    return RZ_OK;
}

int rzt_cancel(struct rzt_timers *t, struct rzt_id id)
{
    if (!t || !kind_valid(t->kind))
    {
        return RZ_INVALID;
    }
    if (id.owner != t || id.slot >= 16 || !t->nodes[id.slot].active)
    {
        return RZ_STALE;
    }
#if RZF_MUTANT != 32
    if (id.generation != t->nodes[id.slot].generation)
    {
        return RZ_STALE;
    }
#endif
    if (t->kind == RZT_HEAP)
    {
        heap_remove(t, (int)id.slot);
    }
    else
    {
        unlink_node(t, (int)id.slot);
    }
    t->nodes[id.slot].active = false;
    --t->count;
    return RZ_OK;
}

static void emit(struct rzt_timers *t, int slot, struct rzt_events *out)
{
    struct rzt_node *n = &t->nodes[slot];
    out->events[out->count++] =
        (struct rzt_event){{t, (unsigned)slot, n->generation},
                           n->due,
                           n->sequence,
                           t->observed,
                           t->observed - n->due};
    n->active = false;
    --t->count;
}

int rzt_advance(struct rzt_timers *t, uint64_t target, struct rzt_events *out)
{
    if (!t || !out || !kind_valid(t->kind))
    {
        return RZ_INVALID;
    }
    if (target < t->now || target - t->now > 1024)
    {
        return RZ_RANGE;
    }
    struct rzt_events events = {0};
    if (target > t->observed)
    {
        t->observed = target;
    }
    if (t->kind != RZT_WHEEL)
    {
        int slot = t->kind == RZT_LIST ? t->heads[0]
                   : t->heap_count     ? t->heap[0]
                                       : -1;
        while (slot >= 0 && t->nodes[slot].due <= target)
        {
            if (t->kind == RZT_LIST)
            {
                unlink_node(t, slot);
            }
            else
            {
                heap_remove(t, slot);
            }
            emit(t, slot, &events);
            slot = t->kind == RZT_LIST ? t->heads[0]
                   : t->heap_count     ? t->heap[0]
                                       : -1;
        }
        t->now = target;
    }
    else
    {
        while (t->now < target)
        {
            ++t->now;
            WORK_ADD(t, ticks, 1);
#if RZF_MUTANT != 31
            if (!(t->now & 7u))
            {
                int bucket = 8 + (int)((t->now / 8) & 7u),
                    slot = t->heads[bucket];
                while (slot >= 0)
                {
                    int next = t->nodes[slot].next;
                    WORK_ADD(t, cascade_visits, 1);
                    if (t->nodes[slot].due - t->now < 8)
                    {
                        unlink_node(t, slot);
                        int fine = (int)(t->nodes[slot].due & 7u);
                        link_before(t, slot, fine, t->heads[fine]);
                    }
                    slot = next;
                }
            }
#endif
            int bucket = (int)(t->now & 7u), slots[16];
            unsigned count = 0;
            int slot = t->heads[bucket];
            while (slot >= 0)
            {
                int next = t->nodes[slot].next;
                if (t->nodes[slot].due == t->now)
                {
                    unlink_node(t, slot);
                    unsigned at = count++;
                    while (at && less(t, slot, slots[at - 1]))
                    {
                        slots[at] = slots[at - 1];
                        --at;
                    }
                    slots[at] = slot;
                }
                slot = next;
            }
            for (unsigned i = 0; i < count; ++i)
            {
                emit(t, slots[i], &events);
            }
        }
    }
#ifndef RZT_NO_WORK
    if (events.count > t->work.largest_expiry)
    {
        t->work.largest_expiry = events.count;
    }
#endif
    *out = events;
    return RZ_OK;
}

int rzt_invariant(const struct rzt_timers *t)
{
    if (!t || !kind_valid(t->kind) || t->count > 16 || t->heap_count > 16 ||
        t->observed < t->now)
    {
        return 0;
    }
    unsigned seen = 0, count = 0;
    if (t->kind == RZT_HEAP)
    {
        if (t->heap_count != t->count)
        {
            return 0;
        }
        for (unsigned i = 0; i < t->heap_count; ++i)
        {
            int slot = t->heap[i];
            if (slot < 0 || slot >= 16 || (seen & (1u << slot)) ||
                !t->nodes[slot].active || t->nodes[slot].position != (int)i)
            {
                return 0;
            }
            seen |= 1u << slot;
            ++count;
            if (i)
            {
                const struct rzt_node *child = &t->nodes[slot],
                                      *parent = &t->nodes[t->heap[(i - 1) / 2]];
                if (child->due < parent->due ||
                    (child->due == parent->due &&
                     child->sequence < parent->sequence))
                {
                    return 0;
                }
            }
        }
    }
    else
    {
        for (int bucket = 0; bucket < 16; ++bucket)
        {
            int previous = -1, slot = t->heads[bucket];
            if (t->kind == RZT_LIST && bucket && slot != -1)
            {
                return 0;
            }
            while (slot >= 0)
            {
                if (slot >= 16 || (seen & (1u << slot)))
                {
                    return 0;
                }
                const struct rzt_node *n = &t->nodes[slot];
                if (!n->active || n->previous != previous ||
                    n->bucket != bucket)
                {
                    return 0;
                }
                if (t->kind == RZT_WHEEL && bucket < 8 &&
                    (int)(n->due & 7u) != bucket)
                {
                    return 0;
                }
                if (t->kind == RZT_WHEEL && bucket >= 8 &&
                    8 + (int)((n->due / 8) & 7u) != bucket)
                {
                    return 0;
                }
                if (t->kind == RZT_LIST && previous >= 0)
                {
                    const struct rzt_node *p = &t->nodes[previous];
                    if (n->due < p->due ||
                        (n->due == p->due && n->sequence < p->sequence))
                    {
                        return 0;
                    }
                }
                seen |= 1u << slot;
                ++count;
                previous = slot;
                slot = n->next;
            }
        }
    }
    if (count != t->count)
    {
        return 0;
    }
    for (unsigned i = 0; i < 16; ++i)
    {
        if (t->nodes[i].active != ((seen & (1u << i)) != 0))
        {
            return 0;
        }
    }
    return 1;
}
