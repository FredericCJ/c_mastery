#include "core.h"

int cf_ring_init(struct cf_ring *q, unsigned storage, unsigned limit,
                 unsigned bits)
{
    if (!q || !storage || storage > 32 || (storage & (storage - 1)) || !limit ||
        limit > storage || (bits != 8 && bits != 32) ||
        (bits == 8 && limit >= 128))
        return CF_INVALID;
    atomic_init(&q->head, 0);
    atomic_init(&q->tail, 0);
    q->storage = storage;
    q->limit = limit;
    q->mask = bits == 8 ? 255u : UINT_MAX;
    return CF_OK;
}

int cf_push(struct cf_ring *q, const struct cf_value *v, unsigned n,
            cf_hook_fn hook, void *ctx)
{
    if (!q || !v || !n || n > q->limit)
        return CF_INVALID;
    unsigned h = atomic_load_explicit(&q->head, memory_order_relaxed);
    unsigned t = atomic_load_explicit(&q->tail, memory_order_acquire);
    unsigned used = (h - t) & q->mask;
    if (used > q->limit || n > q->limit - used)
        return CF_FULL;
    for (unsigned i = 0; i < n; i++)
    {
        struct cf_value *d = &q->values[(h + i) & (q->storage - 1)];
        d->sequence = v[i].sequence;
        d->inverse = v[i].inverse;
    }
#ifdef CF_MUTANT_PUBLISH
    atomic_store_explicit(&q->head, (h + n) & q->mask, memory_order_release);
#endif
    if (hook)
        hook(ctx);
#ifndef CF_MUTANT_PUBLISH
    atomic_store_explicit(&q->head, (h + n) & q->mask, memory_order_release);
#endif
    return CF_OK;
}

int cf_pop(struct cf_ring *q, struct cf_value *v, unsigned n, cf_hook_fn hook,
           void *ctx)
{
    if (!q || !v || !n || n > q->limit)
        return CF_INVALID;
    unsigned t = atomic_load_explicit(&q->tail, memory_order_relaxed);
    unsigned h = atomic_load_explicit(&q->head, memory_order_acquire);
    if (((h - t) & q->mask) < n)
        return CF_EMPTY;
    for (unsigned i = 0; i < n; i++)
    {
        const struct cf_value *s = &q->values[(t + i) & (q->storage - 1)];
        v[i].sequence = s->sequence;
        v[i].inverse = s->inverse;
    }
#ifdef CF_MUTANT_REUSE
    atomic_store_explicit(&q->tail, (t + n) & q->mask, memory_order_release);
#endif
    if (hook)
        hook(ctx);
#ifndef CF_MUTANT_REUSE
    atomic_store_explicit(&q->tail, (t + n) & q->mask, memory_order_release);
#endif
    return CF_OK;
}

int cf_increment(atomic_uint *value, unsigned *out, unsigned *attempts,
                 unsigned spurious)
{
    if (!value || !out || !attempts || spurious > 16)
        return CF_INVALID;
    unsigned old = atomic_load_explicit(value, memory_order_relaxed), count = 0;
    for (;;)
    {
        if (old == UINT_MAX)
        {
            *attempts = count;
            return CF_EXHAUSTED;
        }
        count++;
        if (spurious)
        {
            spurious--;
            old = atomic_load_explicit(value, memory_order_relaxed);
            continue;
        }
        if (atomic_compare_exchange_weak_explicit(value, &old, old + 1,
                                                  memory_order_relaxed,
                                                  memory_order_relaxed))
        {
            *out = old;
            *attempts = count;
            return CF_OK;
        }
    }
}

static unsigned tagged(unsigned g, unsigned state)
{
    return (g << 3) | state;
}

static int valid(const struct cf_pool *p, struct cf_ticket t)
{
    return p && t.owner == p && t.index > 0 && t.index < 32 &&
           t.generation > 0 && t.generation <= p->generation_limit;
}

int cf_pool_init(struct cf_pool *p, unsigned limit)
{
    if (!p || !limit || limit > (UINT_MAX >> 3))
        return CF_INVALID;
    p->generation_limit = limit;
    for (unsigned i = 0; i < 32; i++)
        atomic_init(&p->state[i], tagged(1, i ? CF_FREE : CF_LINKED));
    return CF_OK;
}

int cf_take(struct cf_pool *p, unsigned worker, unsigned steal,
            struct cf_ticket *out, unsigned *scans)
{
    if (!p || worker >= 4 || steal > 1 || !out || !scans)
        return CF_INVALID;
    unsigned count = 0;
    for (unsigned i = 0; i < (steal ? 32u : 8u); i++)
    {
        unsigned n = (8 * worker + i) % 32;
        if (!n)
            continue;
        count++;
        unsigned old = atomic_load_explicit(&p->state[n], memory_order_relaxed);
        if ((old & 7u) == CF_FREE &&
            atomic_compare_exchange_strong_explicit(
                &p->state[n], &old, (old & ~7u) | CF_RESERVED,
                memory_order_acq_rel, memory_order_relaxed))
        {
            out->owner = p;
            out->index = n;
            out->generation = old >> 3;
            *scans = count;
            return CF_OK;
        }
    }
    *scans = count;
    return CF_FULL;
}

static int transition(struct cf_pool *p, struct cf_ticket t, unsigned from,
                      unsigned to)
{
    if (!valid(p, t))
        return CF_INVALID;
    unsigned old = tagged(t.generation, from);
    return atomic_compare_exchange_strong_explicit(
               &p->state[t.index], &old, tagged(t.generation, to),
               memory_order_acq_rel, memory_order_acquire)
               ? CF_OK
               : CF_STALE;
}

static int release(struct cf_pool *p, struct cf_ticket t, unsigned from)
{
    if (!valid(p, t))
        return CF_INVALID;
    unsigned old = tagged(t.generation, from);
    unsigned next = t.generation == p->generation_limit
                        ? tagged(t.generation, CF_RETIRED_FOREVER)
                        : tagged(t.generation + 1, CF_FREE);
    return atomic_compare_exchange_strong_explicit(&p->state[t.index], &old,
                                                   next, memory_order_acq_rel,
                                                   memory_order_acquire)
               ? CF_OK
               : CF_STALE;
}

int cf_publish(struct cf_pool *p, struct cf_ticket t)
{
    return transition(p, t, CF_RESERVED, CF_LINKED);
}

int cf_cancel(struct cf_pool *p, struct cf_ticket t)
{
#ifdef CF_MUTANT_CANCEL_LINKED
    if (valid(p, t) &&
        atomic_load(&p->state[t.index]) == tagged(t.generation, CF_LINKED))
        return release(p, t, CF_LINKED);
#endif
    return release(p, t, CF_RESERVED);
}

int cf_retire(struct cf_pool *p, struct cf_ticket t)
{
    return transition(p, t, CF_LINKED, CF_RETIRED);
}

int cf_reclaim(struct cf_pool *p, struct cf_ticket t)
{
    return release(p, t, CF_RETIRED);
}
