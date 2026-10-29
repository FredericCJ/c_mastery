#include "weak.h"
#ifndef MFW_MUTANT
#define MFW_MUTANT 0
#endif
#ifdef MFW_SC
#define MO(x) memory_order_seq_cst
#else
#define MO(x) memory_order_##x
#endif
void mfw_deque_init(struct mfw_deque *q)
{
    atomic_init(&q->top, 1);
    atomic_init(&q->bottom, 1);
    for (size_t i = 0; i < 8; i++)
        atomic_init(&q->slots[i], 0);
}

enum mfw_status mfw_push(struct mfw_deque *q, unsigned value)
{
    size_t b = atomic_load_explicit(&q->bottom, MO(relaxed));
    size_t t = atomic_load_explicit(&q->top, MO(acquire));
    if (b >= SIZE_MAX - 1 && MFW_MUTANT != 22)
        return MFW_OVERFLOW;
    if (b - t >= 8)
        return MFW_FULL;
    atomic_store_explicit(&q->slots[b % 8], value, MO(relaxed));
    if (MFW_MUTANT != 1)
        atomic_thread_fence(MO(release));
    atomic_store_explicit(&q->bottom, b + 1, MO(relaxed));
    return MFW_OK;
}

enum mfw_status mfw_pop(struct mfw_deque *q, unsigned *out)
{
    size_t b = atomic_load_explicit(&q->bottom, MO(relaxed));
    if (!b)
        return MFW_EMPTY;
    --b;
    atomic_store_explicit(&q->bottom, b, MO(relaxed));
    if (MFW_MUTANT != 2)
        atomic_thread_fence(memory_order_seq_cst);
    size_t t = atomic_load_explicit(&q->top, MO(relaxed));
    if (t > b)
    {
        atomic_store_explicit(&q->bottom, b + 1, MO(relaxed));
        return MFW_EMPTY;
    }
    unsigned value = atomic_load_explicit(&q->slots[b % 8], MO(relaxed));
    if (t == b)
    {
        bool won = atomic_compare_exchange_strong_explicit(
            &q->top, &t, t + 1, memory_order_seq_cst, MO(relaxed));
        atomic_store_explicit(&q->bottom, b + 1, MO(relaxed));
        if (!won)
            return MFW_CONFLICT;
    }
    *out = MFW_MUTANT == 21 ? value + 1 : value;
    return MFW_OK;
}

enum mfw_status mfw_steal(struct mfw_deque *q, unsigned *out)
{
    size_t t = atomic_load_explicit(&q->top, MO(acquire));
    if (MFW_MUTANT != 3)
        atomic_thread_fence(memory_order_seq_cst);
    size_t b = atomic_load_explicit(&q->bottom, MO(acquire));
    if (t >= b)
        return MFW_EMPTY;
    unsigned value = atomic_load_explicit(&q->slots[t % 8], MO(relaxed));
    if (!atomic_compare_exchange_strong_explicit(
            &q->top, &t, t + 1, memory_order_seq_cst, MO(relaxed)))
        return MFW_CONFLICT;
    *out = value;
    return MFW_OK;
}

void mfw_sequence_init(struct mfw_sequence *s)
{
    atomic_init(&s->sequence, 0);
    atomic_init(&s->first, 0);
    atomic_init(&s->second, 0);
}

enum mfw_status mfw_write(struct mfw_sequence *s, unsigned x, unsigned y)
{
    unsigned v = atomic_load_explicit(&s->sequence, MO(relaxed));
    if (v & 1u)
        return MFW_BUSY;
    if (v > UINT_MAX - 2)
        return MFW_OVERFLOW;
    atomic_store_explicit(&s->sequence, v + 1, MO(relaxed));
    atomic_store_explicit(&s->first, x,
                          MFW_MUTANT == 11 ? MO(relaxed) : MO(release));
    if (MFW_MUTANT != 32)
        atomic_store_explicit(&s->second, y,
                              MFW_MUTANT == 11 ? MO(relaxed) : MO(release));
    atomic_store_explicit(&s->sequence, v + 2,
                          MFW_MUTANT == 13 ? MO(relaxed) : MO(release));
    return MFW_OK;
}

enum mfw_status mfw_read(struct mfw_sequence *s, size_t tries, unsigned out[2])
{
    if (MFW_MUTANT == 31 && !tries)
        out[0] = 0;
    for (size_t i = 0; i < tries; i++)
    {
        unsigned a = atomic_load_explicit(
            &s->sequence, MFW_MUTANT == 14 ? MO(relaxed) : MO(acquire));
        if (a & 1u)
            continue;
        unsigned x = atomic_load_explicit(&s->first, MO(relaxed));
        unsigned y = atomic_load_explicit(&s->second, MO(relaxed));
        if (MFW_MUTANT != 12)
            atomic_thread_fence(MO(acquire));
        unsigned b = atomic_load_explicit(&s->sequence, MO(relaxed));
        if (a == b)
        {
            out[0] = x;
            out[1] = y;
            return MFW_OK;
        }
    }
    return MFW_BUSY;
}
