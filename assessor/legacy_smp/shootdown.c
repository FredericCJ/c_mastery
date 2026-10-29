#include "shootdown.h"
#include <stddef.h>
#ifndef ST_MUTANT
#define ST_MUTANT 0
#endif
static int st_valid(const struct st_world *w, unsigned cpu)
{
    return w && w->cpus && w->cpus <= ST_CPUS && cpu < w->cpus;
}

static int st_lock(struct st_world *w, struct sl_guard *g, struct sl_stats *s)
{
    *g = (struct sl_guard){0};
    return sl_enter(&w->membership, NULL, g, s) == SL_OK;
}

static unsigned st_pending(const struct st_slot *s)
{
    return atomic_load_explicit(&s->pending, memory_order_acquire);
}

static uint64_t st_generation(const struct st_slot *s)
{
    return atomic_load_explicit(&s->generation, memory_order_acquire);
}

static int st_equal(struct st_request a, struct st_request b)
{
    return a.space == b.space && a.identity == b.identity &&
           a.generation == b.generation && a.targets == b.targets &&
           a.scope == b.scope && a.pages == b.pages && a.first == b.first;
}

int st_init(struct st_world *w, unsigned cpus, uint64_t a, uint64_t b,
            struct st_port port)
{
    if (!w || !cpus || cpus > ST_CPUS || !a || !b || a == b || !port.publish ||
        !port.invalidate || !port.notify)
        return ST_INVALID;
    w->cpus = cpus;
    w->identity[0] = a;
    w->identity[1] = b;
    w->port = port;
    (void)sl_init(&w->membership, SL_TAS, 1, 0);
    for (unsigned cpu = 0; cpu < ST_CPUS; cpu++)
        w->current[cpu] = ST_NO_SPACE;
    for (unsigned space = 0; space < ST_SPACES; space++)
    {
        struct st_slot *s = &w->slot[space];
        atomic_init(&s->pending, 0);
        atomic_init(&s->targets, 0);
        atomic_init(&s->scope, ST_PAGE);
        atomic_init(&s->pages, 0);
        atomic_init(&s->first, 0);
        atomic_init(&s->generation, 0);
        for (unsigned cpu = 0; cpu < ST_CPUS; cpu++)
            atomic_init(&s->ack[cpu], 0);
    }
    return ST_OK;
}

int st_request(const struct st_world *w, unsigned space, struct st_request *out)
{
    if (!w || space >= ST_SPACES || !out)
        return ST_INVALID;
    const struct st_slot *s = &w->slot[space];
    if (!st_pending(s))
        return ST_INCOMPLETE;
    uint64_t generation = st_generation(s);
    struct st_request r = {space,
                           atomic_load(&s->targets),
                           atomic_load(&s->scope),
                           atomic_load(&s->pages),
                           w->identity[space],
                           generation,
                           atomic_load(&s->first)};
    if (!generation || !st_pending(s) || st_generation(s) != generation)
        return ST_STALE;
    *out = r;
    return ST_OK;
}

int st_enter(struct st_world *w, unsigned cpu, unsigned space,
             struct sl_stats *stats)
{
    if (!st_valid(w, cpu) || space >= ST_SPACES)
        return ST_INVALID;
    struct sl_guard guard;
    if (!st_lock(w, &guard, stats))
        return ST_BUSY;
    int result = ST_OK;
    if (w->current[cpu] != ST_NO_SPACE)
        result = ST_BUSY;
    else
    {
        for (unsigned i = 0; i < ST_SPACES; i++)
            if (st_pending(&w->slot[i]) &&
                (i == space || atomic_load(&w->slot[i].scope) == ST_GLOBAL))
                result = ST_BUSY;
        if (result == ST_OK || ST_MUTANT == 4)
        {
            struct st_request r = {
                space, 1u << cpu,          ST_CONTEXT,
                0,     w->identity[space], st_generation(&w->slot[space]),
                0};
            if (!w->port.invalidate(w->port.context, cpu, r))
                result = ST_PORT;
            else
            {
                w->current[cpu] = space;
                result = ST_OK;
            }
        }
    }
    (void)sl_leave(&guard, stats);
    return result;
}

int st_leave(struct st_world *w, unsigned cpu, struct sl_stats *stats)
{
    if (!st_valid(w, cpu))
        return ST_INVALID;
    struct sl_guard guard;
    if (!st_lock(w, &guard, stats))
        return ST_BUSY;
    unsigned space = w->current[cpu];
    int result = space < ST_SPACES ? ST_OK : ST_STALE;
    for (unsigned i = 0; i < ST_SPACES; i++)
        if (st_pending(&w->slot[i]) &&
            (atomic_load(&w->slot[i].targets) & (1u << cpu)))
            result = ST_BUSY;
    if (result == ST_OK)
    {
        struct st_request r = {
            space, 1u << cpu,          ST_CONTEXT,
            0,     w->identity[space], st_generation(&w->slot[space]),
            0};
        if (!w->port.invalidate(w->port.context, cpu, r))
            result = ST_PORT;
        else
            w->current[cpu] = ST_NO_SPACE;
    }
    (void)sl_leave(&guard, stats);
    return result;
}

int st_begin(struct st_world *w, unsigned actor, unsigned space,
             uintptr_t first, unsigned pages, unsigned scope,
             struct st_request *out, struct sl_stats *stats)
{
    if (!st_valid(w, actor) || space >= ST_SPACES || !out || !pages ||
        pages > ST_MAX_PAGES || (first & 4095u) ||
        first > UINTPTR_MAX - (uintptr_t)(pages - 1) * 4096u ||
        scope > ST_GLOBAL)
        return ST_INVALID;
    struct sl_guard guard;
    if (!st_lock(w, &guard, stats))
        return ST_BUSY;
    struct st_slot *s = &w->slot[space];
    int result = ST_OK;
    uint64_t generation = st_generation(s);
    if (st_pending(s))
        result = ST_BUSY;
    for (unsigned i = 0; i < ST_SPACES; i++)
        if (st_pending(&w->slot[i]) &&
            (scope == ST_GLOBAL || atomic_load(&w->slot[i].scope) == ST_GLOBAL))
            result = ST_BUSY;
    if (result == ST_OK && generation == UINT64_MAX)
        result = ST_FULL;
    if (result == ST_OK)
    {
        unsigned targets = 0;
        for (unsigned c = 0; c < w->cpus; c++)
            if ((scope == ST_GLOBAL && ST_MUTANT != 5) ||
                w->current[c] == space)
                targets |= 1u << c;
        struct st_request r = {
            space,          targets, scope, pages, w->identity[space],
            generation + 1, first};
        if (!w->port.publish(w->port.context, r))
            result = ST_PORT;
        else
        {
            atomic_store(&s->targets, targets);
            atomic_store(&s->scope, scope);
            atomic_store(&s->pages, pages);
            atomic_store(&s->first, first);
            atomic_store_explicit(&s->generation, r.generation,
                                  memory_order_release);
            atomic_store_explicit(&s->pending, 1, memory_order_release);
            *out = r;
            if (!w->port.notify(w->port.context, r, targets))
                result = ST_SIGNAL;
        }
    }
    (void)sl_leave(&guard, stats);
    return result;
}

static void st_ack(struct st_slot *slot, unsigned cpu, uint64_t generation)
{
    uint_fast64_t old =
        atomic_load_explicit(&slot->ack[cpu], memory_order_relaxed);
    while (old < generation && !atomic_compare_exchange_weak_explicit(
                                   &slot->ack[cpu], &old, generation,
                                   memory_order_release, memory_order_relaxed))
    {
    }
}

int st_handle(struct st_world *w, unsigned cpu, struct st_request request)
{
    if (!st_valid(w, cpu) || request.space >= ST_SPACES ||
        !(request.targets & (1u << cpu)))
        return ST_INVALID;
    struct st_request current;
    int read = st_request(w, request.space, &current);
    if (read != ST_OK)
        return read;
    if (!st_equal(current, request))
        return ST_STALE;
    struct st_slot *slot = &w->slot[request.space];
    if (ST_MUTANT == 1)
        st_ack(slot, cpu, request.generation);
    if (!w->port.invalidate(w->port.context, cpu, request))
        return ST_PORT;
    if (st_request(w, request.space, &current) != ST_OK ||
        !st_equal(current, request))
        return ST_STALE;
    st_ack(slot, cpu, request.generation);
    return ST_OK;
}

static unsigned st_outstanding(const struct st_world *w, struct st_request r)
{
    unsigned remaining = r.targets;
    for (unsigned cpu = 0; cpu < w->cpus; cpu++)
        if ((remaining & (1u << cpu)) &&
            (ST_MUTANT == 2
                 ? atomic_load_explicit(&w->slot[r.space].ack[cpu],
                                        memory_order_acquire) != 0
                 : atomic_load_explicit(&w->slot[r.space].ack[cpu],
                                        memory_order_acquire) == r.generation))
            remaining &= ~(1u << cpu);
    return remaining;
}

int st_retry(struct st_world *w, struct st_request r, struct sl_stats *stats)
{
    if (!w || r.space >= ST_SPACES)
        return ST_INVALID;
    struct sl_guard guard;
    if (!st_lock(w, &guard, stats))
        return ST_BUSY;
    struct st_request current;
    int result = st_request(w, r.space, &current);
    if (result == ST_OK && !st_equal(r, current))
        result = ST_STALE;
    if (result == ST_OK)
    {
        unsigned remaining = st_outstanding(w, r);
        if (remaining && !w->port.notify(w->port.context, r, remaining))
            result = ST_SIGNAL;
    }
    (void)sl_leave(&guard, stats);
    return result;
}

int st_finish(struct st_world *w, struct st_request r, int expired,
              struct st_permit *permit, struct sl_stats *stats)
{
    if (!w || r.space >= ST_SPACES || !permit || (expired != 0 && expired != 1))
        return ST_INVALID;
    struct sl_guard guard;
    if (!st_lock(w, &guard, stats))
        return ST_BUSY;
    struct st_request current;
    int result = st_request(w, r.space, &current);
    if (result == ST_OK && !st_equal(current, r))
        result = ST_STALE;
    if (result == ST_OK)
    {
        unsigned missing = st_outstanding(w, r);
        if (missing && !(expired && ST_MUTANT == 3))
            result = expired ? ST_TIMEOUT : ST_INCOMPLETE;
        else
        {
            atomic_store_explicit(&w->slot[r.space].pending, 0,
                                  memory_order_release);
            *permit = (struct st_permit){r};
        }
    }
    (void)sl_leave(&guard, stats);
    return result;
}
