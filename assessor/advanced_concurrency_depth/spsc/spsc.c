#include "spsc.h"

static void copy_value(struct as_value *out, const struct as_value *in)
{
    out->sequence = in->sequence;
    out->inverse = in->inverse;
    out->salt = in->salt;
    out->triple = in->triple;
}

static int valid(unsigned n)
{
    return n && n <= 16 && !(n & (n - 1));
}

enum cmx_result as_init(struct as_queue *q, unsigned capacity)
{
    if (!valid(capacity))
        return CMX_INVALID;
#ifdef AS_MUTANT_INTERFACE
    capacity = 1;
#endif
    atomic_init(&q->head, 0);
    atomic_init(&q->tail, 0);
    q->capacity = capacity;
    q->producer_active = q->consumer_active = 0;
    return CMX_OK;
}

enum cmx_result as_push(struct as_queue *q, struct as_value value,
                        as_hook_fn hook, void *context)
{
    if (q->producer_active)
        return CMX_INVALID;
    q->producer_active = 1;
    unsigned h = atomic_load_explicit(&q->head, memory_order_relaxed);
    unsigned t = atomic_load_explicit(&q->tail, memory_order_acquire);
#ifdef AS_MUTANT_WRAP
    if (h < t)
    {
        q->producer_active = 0;
        return CMX_FULL;
    }
#endif
    if ((unsigned)(h - t) >= q->capacity)
    {
        q->producer_active = 0;
        return CMX_FULL;
    }
    copy_value(&q->data[h & (q->capacity - 1)], &value);
#ifdef AS_MUTANT_PUBLISH
    atomic_store_explicit(&q->head, h + 1, memory_order_release);
#endif
    if (hook)
        hook(context);
#ifndef AS_MUTANT_PUBLISH
    atomic_store_explicit(&q->head, h + 1, memory_order_release);
#endif
    q->producer_active = 0;
    return CMX_OK;
}

enum cmx_result as_pop(struct as_queue *q, struct as_value *out,
                       as_hook_fn hook, void *context)
{
    if (!out || q->consumer_active)
        return CMX_INVALID;
    q->consumer_active = 1;
    unsigned t = atomic_load_explicit(&q->tail, memory_order_relaxed);
    unsigned h = atomic_load_explicit(&q->head, memory_order_acquire);
    if (t == h)
    {
        q->consumer_active = 0;
        return CMX_RETRY;
    }
    struct as_value value;
    copy_value(&value, &q->data[t & (q->capacity - 1)]);
#ifdef AS_MUTANT_REUSE
    atomic_store_explicit(&q->tail, t + 1, memory_order_release);
#endif
    if (hook)
        hook(context);
    copy_value(out, &value);
#ifndef AS_MUTANT_REUSE
    atomic_store_explicit(&q->tail, t + 1, memory_order_release);
#endif
    q->consumer_active = 0;
    return CMX_OK;
}

enum cmx_result as_circular_init(struct as_circular *q, unsigned n)
{
    if (!valid(n))
        return CMX_INVALID;
    atomic_init(&q->head, 0);
    atomic_init(&q->tail, 0);
    q->storage = n;
    return CMX_OK;
}

enum cmx_result as_circular_push(struct as_circular *q, struct as_value value)
{
    unsigned h = atomic_load_explicit(&q->head, memory_order_relaxed);
    unsigned next = (h + 1) & (q->storage - 1);
    unsigned t = atomic_load_explicit(&q->tail, memory_order_acquire);
#ifdef AS_MUTANT_CIRCULAR
    if (h == t)
        return CMX_FULL;
#else
    if (next == t)
        return CMX_FULL;
#endif
    copy_value(&q->data[h], &value);
    atomic_store_explicit(&q->head, next, memory_order_release);
    return CMX_OK;
}

enum cmx_result as_circular_pop(struct as_circular *q, struct as_value *out)
{
    if (!out)
        return CMX_INVALID;
    unsigned t = atomic_load_explicit(&q->tail, memory_order_relaxed);
    unsigned h = atomic_load_explicit(&q->head, memory_order_acquire);
    if (h == t)
        return CMX_RETRY;
    copy_value(out, &q->data[t]);
    atomic_store_explicit(&q->tail, (t + 1) & (q->storage - 1),
                          memory_order_release);
    return CMX_OK;
}
