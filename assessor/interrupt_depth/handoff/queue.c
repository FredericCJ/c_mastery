#include "handoff.h"
#ifndef XI_MUTANT
#define XI_MUTANT 0
#endif
_Static_assert(sizeof(unsigned) == 4 && UINT_MAX == UINT32_MAX,
               "declared32-bit index");

static void clear_record(struct xi_record *r)
{
    r->sequence = r->timestamp = r->original_length = r->copied = r->flags = 0;
    for (unsigned i = 0; i < XI_RECORD_BYTES; ++i)
    {
        r->bytes[i] = 0;
    }
}

static void copy_record(struct xi_record *d, const struct xi_record *s)
{
    d->sequence = s->sequence;
    d->timestamp = s->timestamp;
    d->original_length = s->original_length;
    d->copied = s->copied;
    d->flags = s->flags;
    for (unsigned i = 0; i < XI_RECORD_BYTES; ++i)
    {
        d->bytes[i] = s->bytes[i];
    }
}

int xi_init(struct xi_queue *q, unsigned slots)
{
    if (!q || slots < 2 || slots > XI_QUEUE_SLOTS || (slots & (slots - 1)))
    {
        return XI_INVALID;
    }
    if (ATOMIC_INT_LOCK_FREE != 2)
    {
        return XI_UNSUPPORTED;
    }
    atomic_init(&q->head, 0);
    atomic_init(&q->tail, 0);
    q->slots = slots;
    q->p_phase = q->p_index = q->c_active = q->c_index = 0;
    for (unsigned i = 0; i < XI_QUEUE_SLOTS; ++i)
    {
        clear_record(&q->records[i]);
    }
    return XI_OK;
}

int xi_reserve(struct xi_queue *q)
{
    if (!q)
    {
        return XI_INVALID;
    }
    if (q->p_phase && XI_MUTANT != 4)
    {
        return XI_BUSY;
    }
    unsigned h = atomic_load_explicit(&q->head, memory_order_relaxed);
    unsigned t = atomic_load_explicit(&q->tail, memory_order_acquire);
    if (((h + 1) & (q->slots - 1)) == t)
    {
        return XI_FULL;
    }
    q->p_index = h;
    q->p_phase = 1;
    if (XI_MUTANT == 1)
    {
        atomic_store_explicit(&q->head, (h + 1) & (q->slots - 1),
                              memory_order_release);
    }
    return XI_OK;
}

int xi_fill(struct xi_queue *q, const struct xi_record *r)
{
    if (!q || !r || r->copied > XI_RECORD_BYTES)
    {
        return XI_INVALID;
    }
    if (q->p_phase != 1)
    {
        return XI_STATE;
    }
    copy_record(&q->records[q->p_index], r);
    q->p_phase = 2;
    return XI_OK;
}

int xi_commit(struct xi_queue *q)
{
    if (!q)
    {
        return XI_INVALID;
    }
    if (q->p_phase != 2)
    {
        return XI_STATE;
    }
    atomic_store_explicit(&q->head, (q->p_index + 1) & (q->slots - 1),
                          XI_MUTANT == 7 ? memory_order_relaxed
                                         : memory_order_release);
    q->p_phase = 0;
    return XI_OK;
}

int xi_cancel(struct xi_queue *q)
{
    if (!q)
    {
        return XI_INVALID;
    }
    if (!q->p_phase)
    {
        return XI_STATE;
    }
    q->p_phase = 0;
    return XI_OK;
}

int xi_take(struct xi_queue *q, struct xi_record *out)
{
    if (!q || !out)
    {
        return XI_INVALID;
    }
    if (q->c_active)
    {
        return XI_BUSY;
    }
    unsigned t = atomic_load_explicit(&q->tail, memory_order_relaxed),
             h = atomic_load_explicit(&q->head, memory_order_acquire);
    if (t == h)
    {
        return XI_EMPTY;
    }
    q->c_index = t;
    q->c_active = 1;
    copy_record(out, &q->records[t]);
    if (XI_MUTANT == 2)
    {
        atomic_store_explicit(&q->tail, (t + 1) & (q->slots - 1),
                              memory_order_release);
    }
    return XI_OK;
}

int xi_release(struct xi_queue *q)
{
    if (!q)
    {
        return XI_INVALID;
    }
    if (!q->c_active)
    {
        return XI_STATE;
    }
    atomic_store_explicit(&q->tail, (q->c_index + 1) & (q->slots - 1),
                          XI_MUTANT == 8 ? memory_order_relaxed
                                         : memory_order_release);
    q->c_active = 0;
    return XI_OK;
}

int xi_push(struct xi_queue *q, const struct xi_record *r)
{
    if (!r || r->copied > XI_RECORD_BYTES)
    {
        return XI_INVALID;
    }
    int rc = xi_reserve(q);
    if (rc)
    {
        return rc;
    }
    rc = xi_fill(q, r);
    if (rc)
    {
        (void)xi_cancel(q);
        return rc;
    }
    return xi_commit(q);
}

int xi_pop(struct xi_queue *q, struct xi_record *r)
{
    int rc = xi_take(q, r);
    return rc ? rc : xi_release(q);
}
