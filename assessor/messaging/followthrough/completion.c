#include "completion.h"
#include <stdlib.h>
#ifndef MFX_MUTANT
#define MFX_MUTANT 0
#endif
enum
{
    CANCEL = 1,
    CLAIMED = 2
};

static void receive(void *p, enum msg_status status, unsigned ignored)
{
    struct mfx_completion *c = p;
    mfo_receiver_fn receiver = c->receiver;
    void *context = c->context;
    struct mfo_value owner = c->pending;
    (void)ignored;
    c->pending = (struct mfo_value){0};
    receiver(context, status, owner);
}

enum msg_status mfx_create(struct mfx_completion **out,
                           mfo_receiver_fn receiver, void *context,
                           void (*released)(void *), void *released_context,
                           bool fail)
{
    if (!out || !receiver)
        return MSG_INVALID;
    if (fail)
        return MSG_FAILED;
    struct mfx_completion *c = malloc(sizeof(*c));
    if (!c)
        return MSG_FAILED;
    atomic_init(&c->references, 1);
    atomic_init(&c->flags, 0);
    c->receiver = receiver;
    c->context = context;
    c->pending = (struct mfo_value){0};
    c->released = released;
    c->released_context = released_context;
    msg_connect(&c->operation, receive, c);
    if (msg_start(&c->operation) != MSG_OK)
    {
        free(c);
        return MSG_FAILED;
    }
    *out = c;
    return MSG_OK;
}

enum msg_status mfx_retain(struct mfx_completion *c,
                           struct mfx_completion **out)
{
    if (!c || !out)
        return MSG_INVALID;
    size_t count = atomic_load_explicit(&c->references, memory_order_relaxed);
    do
    {
        if (count == SIZE_MAX)
            return MSG_OVERFLOW;
        if (MFX_MUTANT == 112)
            break;
    } while (!atomic_compare_exchange_weak_explicit(
        &c->references, &count, count + 1, memory_order_relaxed,
        memory_order_relaxed));
    *out = c;
    return MSG_OK;
}

void mfx_drop(struct mfx_completion **token)
{
    if (!token || !*token)
        return;
    struct mfx_completion *c = *token;
    *token = NULL;
    if (atomic_fetch_sub_explicit(&c->references, 1, memory_order_acq_rel) == 1)
    {
        void (*released)(void *) = c->released;
        void *context = c->released_context;
        free(c);
        if (released)
            released(context);
    }
}

enum msg_status mfx_cancel(struct mfx_completion *c)
{
    if (!c)
        return MSG_INVALID;
    unsigned before =
        atomic_fetch_or_explicit(&c->flags, CANCEL, memory_order_acq_rel);
    return before & CLAIMED ? MSG_CLOSED : MSG_OK;
}

enum msg_status mfx_complete(struct mfx_completion *c, enum msg_status status,
                             struct mfo_value *owner)
{
    if (!c || !owner || (owner->data && !owner->drop) ||
        (status != MSG_OK && status != MSG_FAILED && status != MSG_CANCELLED))
        return MSG_INVALID;
    unsigned flags = atomic_load_explicit(&c->flags, memory_order_acquire);
    do
    {
        if ((flags & CLAIMED) && MFX_MUTANT != 111)
            return MSG_BUSY;
    } while (!atomic_compare_exchange_weak_explicit(
        &c->flags, &flags, flags | CLAIMED, memory_order_acq_rel,
        memory_order_acquire));
    /* Only the winner accesses the ordinary operation and receiver context. */
    if (flags & CANCEL)
        status = MSG_CANCELLED;
    c->pending = *owner;
    *owner = (struct mfo_value){0};
    return msg_complete(&c->operation, status, 0);
}
