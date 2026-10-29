#include "bridge.h"

void mb_region_init(struct mb_region *r)
{
    if (r)
    {
        r->cursor = r->payload = r->high = r->count = 0;
        r->generation = 1;
        r->busy = false;
    }
}

int mb_region_alloc(struct mb_region *r, size_t n, size_t a,
                    struct mb_span *out)
{
    if (!r || !out || !n || !a || a > 8 || a > _Alignof(max_align_t) ||
        (a & (a - 1)))
        return MB_INVALID;
    if (r->busy)
        return MB_BUSY;
    if (r->generation > 3)
        return MB_FULL;
    size_t padding = (a - r->cursor % a) % a;
    if (padding > 32 - r->cursor || n > 32 - r->cursor - padding)
        return MB_FULL;
    size_t offset = r->cursor + padding;
    r->cursor = offset + n;
    r->payload += n;
    if (r->cursor > r->high)
        r->high = r->cursor;
    *out = (struct mb_span){r, r->generation, offset, n};
    return MB_OK;
}

int mb_region_cleanup(struct mb_region *r, mb_cleanup_fn f, void *c)
{
    if (!r || !f)
        return MB_INVALID;
    if (r->busy)
        return MB_BUSY;
    if (r->count == 4 || r->generation > 3)
        return MB_FULL;
    r->cleanup[r->count] = f;
    r->context[r->count++] = c;
    return MB_OK;
}

int mb_region_reset(struct mb_region *r)
{
    if (!r)
        return MB_INVALID;
    if (r->busy)
        return MB_BUSY;
    r->busy = true;
    while (r->count)
    {
        r->count--;
#ifndef MB_MUTATE_REGION1
        r->cleanup[r->count](r->context[r->count]);
#endif
    }
    r->cursor = r->payload = 0;
#ifndef MB_MUTATE_REGION2
    if (r->generation <= 3)
        r->generation++;
#endif
    r->busy = false;
    return MB_OK;
}

int mb_region_read(const struct mb_region *r, struct mb_span s, size_t at,
                   unsigned char *out)
{
    if (!r || !out)
        return MB_INVALID;
    if (s.owner != r || s.generation != r->generation || r->generation > 3)
        return MB_STALE;
    if (at >= s.length || s.offset > r->cursor ||
        s.length > r->cursor - s.offset)
        return MB_INVALID;
    *out = r->bytes[s.offset + at];
    return MB_OK;
}

void mb_once_init(struct mb_once *o, bool retry)
{
    if (o)
    {
        atomic_init(&o->state, 0);
        o->retry = retry;
        o->values[0] = o->values[1] = 0;
    }
}

int mb_once_enter(struct mb_once *o, unsigned reader, unsigned *out)
{
    if (!o || !out || reader >= 3)
        return MB_INVALID;
    unsigned state = atomic_load_explicit(&o->state, memory_order_acquire);
    if (state == 4)
    {
        out[0] = o->values[0];
        out[1] = o->values[1];
        return MB_OK;
    }
    if (state == 5)
        return MB_FAILED;
    if (state)
        return state == reader + 1 ? MB_RECURSIVE : MB_BUSY;
    unsigned expected = 0;
#ifdef MB_MUTATE_ONCE1
    unsigned desired = 4;
#else
    unsigned desired = reader + 1;
#endif
    if (atomic_compare_exchange_strong_explicit(&o->state, &expected, desired,
                                                memory_order_acq_rel,
                                                memory_order_acquire))
        return MB_INIT;
    return MB_RETRY;
}

int mb_once_finish(struct mb_once *o, unsigned reader, bool success,
                   unsigned first, unsigned second)
{
    if (!o || reader >= 3)
        return MB_INVALID;
    if (atomic_load(&o->state) != reader + 1)
        return MB_STALE;
    if (success)
    {
        o->values[0] = first;
        o->values[1] = second;
        atomic_store_explicit(&o->state, 4, memory_order_release);
    }
    else
    {
#ifdef MB_MUTATE_ONCE2
        atomic_store_explicit(&o->state, 0, memory_order_release);
#else
        atomic_store_explicit(&o->state, o->retry ? 0 : 5,
                              memory_order_release);
#endif
    }
    return MB_OK;
}

void mb_hp_init(struct mb_hazards *h)
{
    if (!h)
        return;
    atomic_init(&h->source, 0);
    for (unsigned i = 0; i < 2; i++)
        atomic_init(&h->hazard[i], 0);
    for (unsigned i = 0; i < 3; i++)
    {
        h->state[i] = 0;
        h->generation[i] = 1;
        h->values[i] = 0;
    }
}

int mb_hp_publish(struct mb_hazards *h, unsigned value)
{
    if (!h)
        return MB_INVALID;
    if (atomic_load(&h->source))
        return MB_BUSY;
    unsigned i;
    for (i = 0; i < 3; i++)
        if (!h->state[i] && h->generation[i] <= 3)
            break;
    if (i == 3)
        return MB_FULL;
    h->values[i] = value;
    h->state[i] = 1;
    atomic_store(&h->source, h->generation[i] * 4 + i + 1);
    return MB_OK;
}

unsigned mb_hp_observe(const struct mb_hazards *h)
{
    return h ? atomic_load(&h->source) : 0;
}

int mb_hp_protect(struct mb_hazards *h, unsigned reader, unsigned candidate,
                  unsigned *out)
{
    if (!h || !out || reader >= 2)
        return MB_INVALID;
    if (!candidate)
        return MB_EMPTY;
    unsigned i = (candidate - 1) % 4;
    if (i >= 3)
        return MB_INVALID;
    atomic_store(&h->hazard[reader], candidate);
#ifndef MB_MUTATE_HAZARD1
    if (atomic_load(&h->source) != candidate)
    {
        atomic_store(&h->hazard[reader], 0);
        return MB_RETRY;
    }
#endif
    *out = h->values[i];
    return MB_OK;
}

void mb_hp_clear(struct mb_hazards *h, unsigned reader)
{
    if (h && reader < 2)
        atomic_store(&h->hazard[reader], 0);
}

int mb_hp_retire(struct mb_hazards *h)
{
    if (!h)
        return MB_INVALID;
    unsigned token = atomic_exchange(&h->source, 0);
    if (!token)
        return MB_EMPTY;
    h->state[(token - 1) % 4] = 2;
    return MB_OK;
}

unsigned mb_hp_collect(struct mb_hazards *h)
{
    if (!h)
        return 0;
    unsigned count = 0;
    for (unsigned i = 0; i < 3; i++)
        if (h->state[i] == 2)
        {
            unsigned token = h->generation[i] * 4 + i + 1;
            bool held = false;
            for (unsigned r = 0; r < 2; r++)
            {
#ifdef MB_MUTATE_HAZARD2
                if (r == 1)
                    continue;
#endif
                if (atomic_load(&h->hazard[r]) == token)
                    held = true;
            }
            if (!held)
            {
                h->state[i] = 0;
                h->generation[i]++;
                count++;
            }
        }
    return count;
}

void mb_epoch_init(struct mb_epoch *e)
{
    if (e)
    {
        e->epoch = 1;
        for (unsigned i = 0; i < 2; i++)
        {
            e->active[i] = false;
            e->reader_epoch[i] = 0;
        }
        for (unsigned i = 0; i < 3; i++)
        {
            e->retired_at[i] = 0;
            e->retired[i] = e->reclaimed[i] = false;
        }
    }
}

int mb_epoch_enter(struct mb_epoch *e, unsigned r)
{
    if (!e || r >= 2)
        return MB_INVALID;
    if (e->active[r])
        return MB_BUSY;
    e->active[r] = true;
    e->reader_epoch[r] = e->epoch;
    return MB_OK;
}

int mb_epoch_leave(struct mb_epoch *e, unsigned r)
{
    if (!e || r >= 2)
        return MB_INVALID;
    if (!e->active[r])
        return MB_STALE;
    e->active[r] = false;
    return MB_OK;
}

int mb_epoch_retire(struct mb_epoch *e, unsigned node)
{
    if (!e || node >= 3)
        return MB_INVALID;
    if (e->retired[node] || e->reclaimed[node])
        return MB_STALE;
    e->retired[node] = true;
    e->retired_at[node] = e->epoch;
    return MB_OK;
}

int mb_epoch_advance(struct mb_epoch *e)
{
    if (!e)
        return MB_INVALID;
    if (e->epoch == 3)
        return MB_FULL;
#ifndef MB_MUTATE_EPOCH1
    for (unsigned r = 0; r < 2; r++)
        if (e->active[r] && e->reader_epoch[r] != e->epoch)
            return MB_BUSY;
#endif
    e->epoch++;
    return MB_OK;
}

unsigned mb_epoch_collect(struct mb_epoch *e)
{
    if (!e)
        return 0;
    unsigned count = 0;
    for (unsigned i = 0; i < 3; i++)
    {
#ifdef MB_MUTATE_EPOCH2
        unsigned distance = 1;
#else
        unsigned distance = 2;
#endif
        if (e->retired[i] && e->epoch - e->retired_at[i] >= distance)
        {
            bool held = false;
            for (unsigned r = 0; r < 2; r++)
                if (e->active[r] && e->reader_epoch[r] <= e->retired_at[i])
                    held = true;
            if (!held)
            {
                e->retired[i] = false;
                e->reclaimed[i] = true;
                count++;
            }
        }
    }
    return count;
}
