#include "queues.h"
#include <stddef.h>
#ifndef SQ_MUTANT
#define SQ_MUTANT 0
#endif
struct ls_hold
{
    struct sl_guard first, second;
    unsigned count;
};

static int valid(const struct sq_world *w, unsigned actor, unsigned q)
{
    return w && w->cpus && w->cpus <= SQ_CPUS && actor < w->cpus && q < w->cpus;
}

static int enter(struct sq_world *w, unsigned a, unsigned b, struct ls_hold *h,
                 struct sl_stats *s)
{
    *h = (struct ls_hold){0};
    if (w->coarse)
    {
        h->count = 1;
        return sl_enter(&w->global, NULL, &h->first, s) == SL_OK;
    }
    if (a > b)
    {
        unsigned x = a;
        a = b;
        b = x;
    }
    h->count = a == b ? 1 : 2;
    if (sl_enter(&w->queue[a].lock, NULL, &h->first, s) != SL_OK)
        return 0;
    if (h->count == 2 &&
        sl_enter(&w->queue[b].lock, NULL, &h->second, s) != SL_OK)
    {
        sl_leave(&h->first, s);
        return 0;
    }
    return 1;
}

static void leave(struct ls_hold *h, struct sl_stats *s)
{
    if (h->count == 2)
        (void)sl_leave(&h->second, s);
    (void)sl_leave(&h->first, s);
}

static unsigned state(const struct sq_task *t)
{
    return atomic_load_explicit(&t->state, memory_order_acquire);
}

static unsigned home(const struct sq_task *t)
{
    return atomic_load_explicit(&t->home, memory_order_acquire);
}

static int matches(const struct sq_task *t, struct sq_handle id, unsigned q)
{
    return id.generation &&
           (SQ_MUTANT == 3 || id.generation == t->generation) && home(t) == q;
}

static void record(struct sq_task *t, unsigned actor, unsigned slot,
                   unsigned old, unsigned next, unsigned src, unsigned dst,
                   struct sq_event *event)
{
    t->revision++;
    atomic_store_explicit(&t->state, next, memory_order_release);
    *event = (struct sq_event){actor, slot, old,           next,
                               src,   dst,  t->generation, t->revision};
}

static unsigned find(const struct sq_queue *q, unsigned slot)
{
    for (unsigned i = 0; i < q->count; i++)
        if (q->slots[i] == slot)
            return i;
    return SQ_NONE;
}

static void remove_at(struct sq_queue *q, unsigned index)
{
    for (unsigned i = index + 1; i < q->count; i++)
        q->slots[i - 1] = q->slots[i];
    q->count--;
}

int sq_init(struct sq_world *w, unsigned cpus, unsigned capacity,
            unsigned coarse)
{
    if (!w || !cpus || cpus > SQ_CPUS || !capacity || capacity > SQ_CAPACITY ||
        coarse > 1)
        return SQ_INVALID;
    w->cpus = cpus;
    w->capacity = capacity;
    w->coarse = coarse;
    (void)sl_init(&w->global, SL_TAS, 1, 0);
    for (unsigned i = 0; i < SQ_CPUS; i++)
    {
        (void)sl_init(&w->queue[i].lock, SL_TAS, i + 1, 0);
        w->queue[i].count = 0;
        w->queue[i].reserved = 0;
        w->queue[i].current = SQ_NONE;
    }
    for (unsigned i = 0; i < SQ_TASKS; i++)
    {
        struct sq_task *t = &w->task[i];
        atomic_init(&t->state, SQ_EMPTY);
        atomic_init(&t->home, 0);
        t->generation = t->revision = 0;
        t->source = t->destination = 0;
    }
    return SQ_OK;
}

int sq_create(struct sq_world *w, unsigned actor, unsigned q, unsigned slot,
              struct sq_handle *id, struct sq_event *event)
{
    if (!valid(w, actor, q) || slot >= SQ_TASKS || !id || !event)
        return SQ_INVALID;
    struct sq_task *t = &w->task[slot];
    unsigned old = state(t);
    if (old != SQ_EMPTY && old != SQ_EXITED)
        return SQ_STATE;
    if (t->generation == UINT64_MAX ||
        w->queue[q].count + w->queue[q].reserved >= w->capacity)
        return SQ_FULL;
    t->generation++;
    t->revision = 0;
    atomic_store(&t->home, q);
    w->queue[q].slots[w->queue[q].count++] = slot;
    record(t, actor, slot, old, SQ_READY, q, q, event);
    *id = (struct sq_handle){slot, t->generation};
    return SQ_OK;
}

int sq_dispatch(struct sq_world *w, unsigned cpu, struct sq_handle *id,
                struct sq_event *event, struct sl_stats *stats)
{
    if (!valid(w, cpu, cpu) || !id || !event)
        return SQ_INVALID;
    struct ls_hold h;
    if (!enter(w, cpu, cpu, &h, stats))
        return SQ_STATE;
    struct sq_queue *q = &w->queue[cpu];
    int result = SQ_OK;
    if (q->current != SQ_NONE)
        result = SQ_STATE;
    else if (!q->count)
        result = SQ_IDLE;
    else
    {
        unsigned slot = q->slots[0];
        struct sq_task *t = &w->task[slot];
        if (home(t) != cpu || state(t) != SQ_READY)
            result = SQ_STATE;
        else if (t->revision > UINT64_MAX - 2)
            result = SQ_FULL;
        else
        {
            if (SQ_MUTANT != 4)
                remove_at(q, 0);
            q->reserved++;
            q->current = slot;
            record(t, cpu, slot, SQ_READY, SQ_RUNNING, cpu, cpu, event);
            *id = (struct sq_handle){slot, t->generation};
        }
    }
    leave(&h, stats);
    return result;
}

int sq_finish(struct sq_world *w, unsigned cpu, struct sq_handle id,
              unsigned next, struct sq_event *event, struct sl_stats *stats)
{
    if (!valid(w, cpu, cpu) || id.slot >= SQ_TASKS || !event ||
        (next != SQ_READY && next != SQ_BLOCKED && next != SQ_EXITED))
        return SQ_INVALID;
    struct ls_hold h;
    if (!enter(w, cpu, cpu, &h, stats))
        return SQ_STATE;
    struct sq_queue *q = &w->queue[cpu];
    struct sq_task *t = &w->task[id.slot];
    int result = SQ_OK;
    if (!matches(t, id, cpu))
        result = SQ_STALE;
    else if (q->current != id.slot || state(t) != SQ_RUNNING)
        result = SQ_STATE;
    else if (!q->reserved)
        result = SQ_STATE;
    else
    {
        q->current = SQ_NONE;
        q->reserved--;
        if (next == SQ_READY)
            q->slots[q->count++] = id.slot;
        record(t, cpu, id.slot, SQ_RUNNING, next, cpu, cpu, event);
    }
    leave(&h, stats);
    return result;
}

int sq_wake(struct sq_world *w, unsigned actor, unsigned qid,
            struct sq_handle id, struct sq_event *event, struct sl_stats *stats)
{
    if (!valid(w, actor, qid) || id.slot >= SQ_TASKS || !event)
        return SQ_INVALID;
    struct ls_hold h;
    if (!enter(w, qid, qid, &h, stats))
        return SQ_STATE;
    struct sq_queue *q = &w->queue[qid];
    struct sq_task *t = &w->task[id.slot];
    unsigned old = state(t);
    int result = SQ_OK;
    if (!matches(t, id, qid))
        result = SQ_STALE;
    else if (old == SQ_READY && SQ_MUTANT != 1)
        result = SQ_DUPLICATE;
    else if (old != SQ_BLOCKED && !(old == SQ_READY && SQ_MUTANT == 1))
        result = SQ_STATE;
    else if (t->revision == UINT64_MAX || q->count + q->reserved >= w->capacity)
        result = SQ_FULL;
    else
    {
        q->slots[q->count++] = id.slot;
        record(t, actor, id.slot, old, SQ_READY, qid, qid, event);
    }
    leave(&h, stats);
    return result;
}

int sq_transfer_begin(struct sq_world *w, unsigned actor, unsigned src,
                      unsigned dst, struct sq_handle id,
                      struct sq_transfer *ticket, struct sq_event *event,
                      struct sl_stats *stats)
{
    if (!valid(w, actor, src) || dst >= w->cpus || src == dst ||
        id.slot >= SQ_TASKS || !ticket || !event)
        return SQ_INVALID;
    struct ls_hold h;
    if (!enter(w, src, dst, &h, stats))
        return SQ_STATE;
    struct sq_task *t = &w->task[id.slot];
    struct sq_queue *a = &w->queue[src], *b = &w->queue[dst];
    unsigned pos = find(a, id.slot);
    int result = SQ_OK;
    if (!matches(t, id, src))
        result = SQ_STALE;
    else if (state(t) != SQ_READY || pos == SQ_NONE)
        result = SQ_STATE;
    else
    {
        if (SQ_MUTANT == 2)
            remove_at(a, pos);
        if (b->count + b->reserved >= w->capacity ||
            t->revision > UINT64_MAX - 2)
            result = SQ_FULL;
        else
        {
            if (SQ_MUTANT != 2)
                remove_at(a, pos);
            a->reserved++;
            b->reserved++;
            t->source = src;
            t->destination = dst;
            record(t, actor, id.slot, SQ_READY, SQ_TRANSFER, src, dst, event);
            *ticket = (struct sq_transfer){id, actor, src, dst, t->revision};
        }
    }
    leave(&h, stats);
    return result;
}

int sq_transfer_end(struct sq_world *w, unsigned actor,
                    struct sq_transfer ticket, int commit,
                    struct sq_event *event, struct sl_stats *stats)
{
    unsigned src = ticket.source, dst = ticket.destination;
    struct sq_handle id = ticket.task;
    if (!valid(w, actor, src) || dst >= w->cpus || src == dst ||
        id.slot >= SQ_TASKS || actor != ticket.actor || !event ||
        (commit != 0 && commit != 1))
        return SQ_INVALID;
    struct ls_hold h;
    if (!enter(w, src, dst, &h, stats))
        return SQ_STATE;
    struct sq_task *t = &w->task[id.slot];
    int result = SQ_OK;
    if (!matches(t, id, src) || state(t) != SQ_TRANSFER ||
        t->revision != ticket.revision || t->source != src ||
        t->destination != dst)
        result = SQ_STALE;
    else
    {
        unsigned target = commit ? dst : src;
        w->queue[src].reserved--;
        w->queue[dst].reserved--;
        w->queue[target].slots[w->queue[target].count++] = id.slot;
        atomic_store_explicit(&t->home, target, memory_order_release);
        record(t, actor, id.slot, SQ_TRANSFER, SQ_READY, src, target, event);
    }
    leave(&h, stats);
    return result;
}

int sq_invariant(const struct sq_world *w)
{
    if (!w || !w->cpus || w->cpus > SQ_CPUS || !w->capacity ||
        w->capacity > SQ_CAPACITY)
        return 0;
    unsigned queued[SQ_TASKS] = {0}, running[SQ_TASKS] = {0},
             reserved[SQ_CPUS] = {0};
    for (unsigned c = 0; c < w->cpus; c++)
    {
        const struct sq_queue *q = &w->queue[c];
        if (q->count + q->reserved > w->capacity || q->count > SQ_CAPACITY)
            return 0;
        for (unsigned j = 0; j < q->count; j++)
        {
            unsigned slot = q->slots[j];
            if (slot >= SQ_TASKS || ++queued[slot] != 1 ||
                home(&w->task[slot]) != c || state(&w->task[slot]) != SQ_READY)
                return 0;
        }
        if (q->current != SQ_NONE)
        {
            if (q->current >= SQ_TASKS || ++running[q->current] != 1 ||
                home(&w->task[q->current]) != c ||
                state(&w->task[q->current]) != SQ_RUNNING)
                return 0;
        }
    }
    for (unsigned i = 0; i < SQ_TASKS; i++)
    {
        const struct sq_task *t = &w->task[i];
        unsigned s = state(t);
        if (s == SQ_READY)
        {
            if (queued[i] != 1 || running[i])
                return 0;
        }
        else if (s == SQ_RUNNING)
        {
            if (running[i] != 1 || queued[i])
                return 0;
            reserved[home(t)]++;
        }
        else if (s == SQ_TRANSFER)
        {
            if (queued[i] || running[i] || t->source >= w->cpus ||
                t->destination >= w->cpus || t->source == t->destination ||
                home(t) != t->source)
                return 0;
            reserved[t->source]++;
            reserved[t->destination]++;
        }
        else if ((s != SQ_EMPTY && s != SQ_BLOCKED && s != SQ_EXITED) ||
                 queued[i] || running[i])
            return 0;
    }
    for (unsigned c = 0; c < w->cpus; c++)
        if (w->queue[c].reserved != reserved[c])
            return 0;
    return 1;
}
