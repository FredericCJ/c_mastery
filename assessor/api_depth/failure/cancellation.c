#include "cancellation.h"

enum ae_code ae_operation_start(struct ae_operation *o, const unsigned char *p,
                                unsigned n, ae_terminal_fn receiver,
                                void *context, unsigned *generation)
{
    if (!o || (!p && n) || n > 4 || !receiver || !generation)
    {
        return AE_INVALID;
    }
    if (o->backend.state == F_INFLIGHT ||
        o->backend.state == F_CANCEL_REQUESTED)
    {
        return AE_CONFLICT;
    }
    if (o->backend.generation >= 3)
    {
        return AE_FULL;
    }
    uint32_t ticket;
    enum f_status s = f_submit(&o->backend, &ticket);
    if (s != F_OK)
    {
        return AE_CONTRACT;
    }
    o->terminal = receiver;
    o->context = context;
    o->length = n;
    o->timed_out = 0;
    o->reported = 0;
    for (unsigned i = 0; i < n; ++i)
    {
        o->payload[i] = p[i];
    }
    *generation = ticket;
    return AE_OK;
}

enum ae_code ae_operation_event(struct ae_operation *o, unsigned generation,
                                enum ae_event event)
{
    if (!o || !generation || (unsigned)event > AE_JOINED)
    {
        return AE_INVALID;
    }
#ifndef AE_MUTANT_STALE
    if (generation != o->backend.generation)
    {
        return AE_CONFLICT;
    }
#endif
    if (o->reported || o->backend.state == F_DONE || o->backend.state == F_IDLE)
    {
        return AE_CONFLICT;
    }
    if (event == AE_DEADLINE)
    {
        if (o->timed_out)
        {
            return AE_AGAIN;
        }
        if (f_cancel(&o->backend, o->backend.generation) != F_PENDING)
        {
            return AE_CONTRACT;
        }
        o->timed_out = 1;
#ifndef AE_MUTANT_EARLY_RELEASE
        return AE_AGAIN;
#endif
    }
    if ((event == AE_CANCEL_ACK || event == AE_JOINED) && !o->timed_out)
    {
        return AE_AGAIN;
    }
    if (f_complete(&o->backend, o->backend.generation) != F_OK)
    {
        return AE_CONTRACT;
    }
    ae_terminal_fn receiver = o->terminal;
    void *context = o->context;
    enum ae_code result = o->timed_out ? AE_TIMEOUT : AE_OK;
    unsigned length = o->length, ticket = o->backend.generation;
    unsigned char payload[4];
    for (unsigned i = 0; i < length; ++i)
    {
        payload[i] = o->payload[i];
    }
    o->reported = 1;
    receiver(context, result, ticket, payload, length);
    /* Receiver may free o. No operation access, including trace hooks, here. */
    return AE_OK;
}
