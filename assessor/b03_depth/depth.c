#include "depth.h"

int b3_pump(struct b3_writer *w, b3_write_fn f, void *c, unsigned budget)
{
    unsigned calls = 0;
    if (!w || !f || (!w->bytes && w->length) || w->position > w->length ||
        budget > 64)
    {
        return B3_INVALID;
    }
    if (w->terminal)
    {
        return w->terminal == 1 ? B3_OK : B3_FAILED;
    }
    if (w->position == w->length)
    {
        w->terminal = 1;
        return B3_OK;
    }
    while (calls++ < budget)
    {
        size_t remaining = w->length - w->position;
        long n = f(c, w->bytes + w->position, remaining);
        if (n > 0)
        {
            if ((unsigned long)n > remaining)
            {
                w->terminal = 2;
                return B3_FAILED;
            }
#ifdef B3_MUTATE_IO
            w->position = w->length;
#else
            w->position += (size_t)n;
#endif
            if (w->position == w->length)
            {
                w->terminal = 1;
                return B3_OK;
            }
        }
        else if (n == -1)
        {
            continue;
        }
        else if (n == -2 || n == 0)
        {
            return B3_WAIT;
        }
        else
        {
            w->terminal = 2;
            return B3_FAILED;
        }
    }
    return B3_YIELD;
}

int b3_start(struct b3_operation *o)
{
    if (!o)
    {
        return B3_INVALID;
    }
    if (o->owned || o->generation >= 3)
    {
        return B3_FAILED;
    }
    o->generation++;
    o->phase = B3_ACTIVE;
    o->owned = 1;
    return B3_OK;
}

int b3_event(struct b3_operation *o, unsigned g, enum b3_event e)
{
    if (!o || e > B3_CANCEL_ACK || e < B3_TIMEOUT)
    {
        return B3_INVALID;
    }
    if (g != o->generation || !o->owned)
    {
        return B3_STALE;
    }
    if (e == B3_TIMEOUT)
    {
        o->phase = B3_CANCELLING;
#ifdef B3_MUTATE_CANCEL
        o->owned = 0;
        o->reports++;
        o->phase = B3_CANCELLED;
#endif
        return B3_WAIT;
    }
    if (e == B3_CANCEL_ACK && o->phase != B3_CANCELLING)
    {
        return B3_STALE;
    }
    o->phase = e == B3_COMPLETE ? B3_COMPLETED : B3_CANCELLED;
    o->owned = 0;
    o->reports++;
    return B3_OK;
}

int b3_ack(struct b3_device *d, unsigned m)
{
    if (!d || m > 15)
    {
        return B3_INVALID;
    }
#ifdef B3_MUTATE_MMIO
    m |= d->status;
#endif
    d->status &= ~m;
    d->writes++;
    return B3_OK;
}

int b3_read_data(struct b3_device *d, unsigned *o)
{
    if (!d || !o)
    {
        return B3_INVALID;
    }
    if (!(d->status & 1u))
    {
        return B3_WAIT;
    }
    *o = d->data;
    d->status &= ~1u;
    d->reads++;
    return B3_OK;
}

int b3_due(uint8_t now, uint8_t start, uint8_t delay, int *out)
{
    unsigned elapsed = (uint8_t)(now - start);
    if (!out || delay >= 128)
    {
        return B3_INVALID;
    }
    if (elapsed >= 128)
    {
        return B3_AMBIGUOUS;
    }
#ifdef B3_MUTATE_DEADLINE
    *out = now >= (unsigned)start + delay;
#else
    *out = elapsed >= delay;
#endif
    return B3_OK;
}

memory_order b3_load_order(void)
{
#ifdef B3_MUTATE_PUBLICATION
    return memory_order_relaxed;
#else
    return memory_order_acquire;
#endif
}

memory_order b3_store_order(void)
{
    return memory_order_release;
}

void b3_mailbox_init(struct b3_mailbox *m)
{
    atomic_init(&m->ready, 0);
    m->payload = 0;
}

int b3_publish(struct b3_mailbox *m, int value)
{
    if (!m)
    {
        return B3_INVALID;
    }
    if (atomic_load_explicit(&m->ready, b3_load_order()))
    {
        return B3_WAIT;
    }
    m->payload = value;
    atomic_store_explicit(&m->ready, 1, b3_store_order());
    return B3_OK;
}

int b3_take(struct b3_mailbox *m, int *out)
{
    if (!m || !out)
    {
        return B3_INVALID;
    }
    if (!atomic_load_explicit(&m->ready, b3_load_order()))
    {
        return B3_WAIT;
    }
    *out = m->payload;
    atomic_store_explicit(&m->ready, 0, b3_store_order());
    return B3_OK;
}
