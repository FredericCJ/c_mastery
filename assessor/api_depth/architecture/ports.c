#include "arch.h"
#ifndef AR_MUTANT
#define AR_MUTANT 0
#endif
static int finish(struct ar_task *t, unsigned status)
{
    (void)t->ops.notify(t->ops.context, 0);
    t->ops.release(t->ops.context, t->data);
    t->data = 0;
    t->active = 0;
    t->terminal = status;
    return (int)status;
}

int ar_begin(struct ar_task *t, struct ar_ops o, const unsigned char *p,
             size_t n, unsigned version, unsigned partial)
{
    if (!t || !p || !n || n > 8 || !o.allocate || !o.release || !o.clock_ms ||
        !o.write || !o.notify || !version || partial > 1)
    {
        return AR_INVALID;
    }
    if (t->active)
    {
        return AR_STATE;
    }
    unsigned char *data = o.allocate(o.context, n);
    if (!data)
    {
        return AR_FAILED;
    }
    for (size_t i = 0; i < n; ++i)
    {
        data[i] = p[i];
    }
    uint64_t now = o.clock_ms(o.context);
    if (o.notify(o.context, 1) != AR_OK)
    {
        o.release(o.context, data);
        return AR_FAILED;
    }
    *t = (struct ar_task){o, data, n,          0,       now,
                          1, 0,    AR_PENDING, version, partial};
    return AR_OK;
}

int ar_poll(struct ar_task *t)
{
    if (!t)
    {
        return AR_INVALID;
    }
    if (!t->active)
    {
        return (int)t->terminal;
    }
    if (t->cancelling)
    {
        return AR_PENDING;
    }
    uint64_t now = t->ops.clock_ms(t->ops.context);
    if (now < t->last_ms)
    {
        return AR_CONTRACT;
    }
    size_t left = t->length - t->offset;
    struct ar_result r =
        t->ops.write(t->ops.context, t->data + t->offset, left);
    if (r.count < 0 || (size_t)r.count > left ||
        (r.state != AR_OK && r.state != AR_PENDING && r.state != AR_FAILED) ||
        (!t->allow_partial_failure && r.state == AR_FAILED && r.count))
    {
        return finish(t, AR_CONTRACT);
    }
    t->last_ms = now;
    if (AR_MUTANT != 3)
    {
        t->offset += (size_t)r.count;
    }
    if (r.state == AR_FAILED)
    {
        return finish(t, AR_FAILED);
    }
    if (t->offset == t->length)
    {
        return finish(t, AR_OK);
    }
    return AR_PENDING;
}

int ar_cancel(struct ar_task *t)
{
    if (!t || !t->active)
    {
        return AR_STATE;
    }
    t->cancelling = 1;
    return AR_PENDING;
}

int ar_cancel_ack(struct ar_task *t)
{
    if (!t || !t->active || !t->cancelling)
    {
        return AR_STATE;
    }
    return finish(t, AR_CANCELLED);
}
