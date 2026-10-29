#include "weak.h"

void mfw_deque_init(struct mfw_deque *q)
{
    atomic_init(&q->top, 1);
    atomic_init(&q->bottom, 1);
    for (size_t i = 0; i < 8; i++)
    {
        atomic_init(&q->slots[i], 0);
    }
}

enum mfw_status mfw_push(struct mfw_deque *q, unsigned v)
{
    (void)q;
    (void)v;
    return MFW_INVALID;
}

enum mfw_status mfw_pop(struct mfw_deque *q, unsigned *out)
{
    (void)q;
    (void)out;
    return MFW_INVALID;
}

enum mfw_status mfw_steal(struct mfw_deque *q, unsigned *out)
{
    (void)q;
    (void)out;
    return MFW_INVALID;
}

void mfw_sequence_init(struct mfw_sequence *s)
{
    atomic_init(&s->sequence, 0);
    atomic_init(&s->first, 0);
    atomic_init(&s->second, 0);
}

enum mfw_status mfw_write(struct mfw_sequence *s, unsigned a, unsigned b)
{
    (void)s;
    (void)a;
    (void)b;
    return MFW_INVALID;
}

enum mfw_status mfw_read(struct mfw_sequence *s, size_t n, unsigned out[2])
{
    (void)s;
    (void)n;
    (void)out;
    return MFW_INVALID;
}
