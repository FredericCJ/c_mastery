#include "ipc.h"

int ei_invariant(const struct ei_request *r)
{
    if (!r || r->epoch < 1 || r->epoch > 65535 || r->request_id < 1 ||
        r->request_id > 65535 || (unsigned)r->state > EI_ABANDONED ||
        (unsigned)r->owner > EI_NO_OWNER ||
        (unsigned)r->outcome > EI_UNCERTAIN || r->connected > 1 ||
        r->effect_done > 1 || r->cancel_requested > 1 || r->cancel_seen > 1 ||
        r->result > 2040)
        return 0;
    if (r->cancel_seen && !r->cancel_requested)
        return 0;
    uint32_t sum = 0;
    for (unsigned i = 0; i < 8; ++i)
        sum += r->payload[i];
    if (r->result != (r->effect_done ? sum : 0))
        return 0;
    if (r->state == EI_ALLOCATED || r->state == EI_QUEUED)
        return r->owner ==
                   (r->state == EI_ALLOCATED ? EI_CLIENT : EI_TRANSPORT) &&
               r->connected && !r->effect_done && !r->cancel_requested &&
               r->outcome == EI_PENDING;
    if (r->state == EI_ACCEPTED)
        return r->owner == EI_SERVER &&
               r->outcome == (r->connected ? EI_PENDING : EI_UNCERTAIN);
    if (r->state == EI_COMPLETED)
        return r->owner == EI_CLIENT && r->connected && r->effect_done &&
               r->outcome == EI_SUCCESS;
    return r->owner == EI_NO_OWNER &&
           r->outcome ==
               (r->effect_done ? EI_UNCERTAIN : EI_DEFINITELY_NOT_DONE);
}

int ei_init(struct ei_request *r, const unsigned char p[8], unsigned epoch,
            unsigned id)
{
    if (!r || !p || epoch < 1 || epoch > 65535 || id < 1 || id > 65535)
        return -1;
    struct ei_request next = {0};
    next.epoch = epoch;
    next.request_id = id;
    next.state = EI_ALLOCATED;
    next.owner = EI_CLIENT;
    next.outcome = EI_PENDING;
    next.connected = 1;
    for (unsigned i = 0; i < 8; ++i)
        next.payload[i] = p[i];
    *r = next;
    return 0;
}

int ei_transition(struct ei_request *r, enum ei_event event)
{
    if (!ei_invariant(r) || (unsigned)event > EI_STOP_CONFIRMED)
        return -1;
    struct ei_request next = *r;
    switch (event)
    {
        case EI_QUEUE:
            if (r->state != EI_ALLOCATED)
                return -1;
            next.state = EI_QUEUED;
            next.owner = EI_TRANSPORT;
            break;
        case EI_ACCEPT:
            if (r->state != EI_QUEUED)
                return -1;
            next.state = EI_ACCEPTED;
            next.owner = EI_SERVER;
            break;
        case EI_EFFECT:
            if (r->state != EI_ACCEPTED || r->effect_done)
                return -1;
            next.effect_done = 1;
            next.result = 0;
            for (unsigned i = 0; i < 8; ++i)
                next.result += next.payload[i];
            break;
        case EI_COMPLETE:
            if (r->state != EI_ACCEPTED || !r->effect_done)
                return -1;
            next.state = r->connected ? EI_COMPLETED : EI_ABANDONED;
            next.owner = r->connected ? EI_CLIENT : EI_NO_OWNER;
            next.outcome = r->connected ? EI_SUCCESS : EI_UNCERTAIN;
            break;
        case EI_DISCONNECT:
            if (!r->connected || r->state == EI_ABANDONED ||
                r->state == EI_COMPLETED)
                return -1;
            next.connected = 0;
            if (r->state == EI_ACCEPTED)
            {
                next.outcome = EI_UNCERTAIN;
            }
            else
            {
                next.state = EI_ABANDONED;
                next.owner = EI_NO_OWNER;
                next.outcome = EI_DEFINITELY_NOT_DONE;
            }
#ifdef EI_MUTANT_EARLY_RELEASE
            if (r->state == EI_ACCEPTED)
            {
                next.state = EI_ABANDONED;
                next.owner = EI_NO_OWNER;
                next.outcome = EI_DEFINITELY_NOT_DONE;
            }
#endif
            break;
        case EI_CANCEL_REQUEST:
            if (r->state != EI_ACCEPTED || r->cancel_requested)
                return -1;
            next.cancel_requested = 1;
            break;
        case EI_CANCEL_SEEN:
            if (r->state != EI_ACCEPTED || !r->cancel_requested ||
                r->cancel_seen)
                return -1;
            next.cancel_seen = 1;
#ifdef EI_MUTANT_ACK_STOPS
            next.state = EI_ABANDONED;
            next.owner = EI_NO_OWNER;
            next.outcome =
                r->effect_done ? EI_UNCERTAIN : EI_DEFINITELY_NOT_DONE;
#endif
            break;
        case EI_STOP_CONFIRMED:
            if (r->state != EI_ACCEPTED || !r->cancel_seen)
                return -1;
            next.state = EI_ABANDONED;
            next.owner = EI_NO_OWNER;
            next.outcome =
                r->effect_done ? EI_UNCERTAIN : EI_DEFINITELY_NOT_DONE;
            break;
    }
    *r = next;
    return 0;
}

static int queue_valid(const struct ei_queue *q)
{
    if (!q || q->head > 3 || q->count > 4 || q->bytes != 8 * q->count)
        return 0;
    for (unsigned i = 0; i < q->count; ++i)
    {
        struct ei_request *r = q->slots[(q->head + i) % 4];
        if (!ei_invariant(r) || r->state != EI_QUEUED)
            return 0;
        for (unsigned j = 0; j < i; ++j)
        {
            struct ei_request *s = q->slots[(q->head + j) % 4];
            if (r == s ||
                (r->epoch == s->epoch && r->request_id == s->request_id))
                return 0;
        }
    }
    return 1;
}

int ei_enqueue(struct ei_queue *q, struct ei_request *r)
{
    if (!queue_valid(q) || !ei_invariant(r) || r->state != EI_ALLOCATED)
        return -1;
    for (unsigned i = 0; i < q->count; ++i)
    {
        struct ei_request *s = q->slots[(q->head + i) % 4];
        if (s == r || (s->epoch == r->epoch && s->request_id == r->request_id))
            return -1;
    }
    if (q->count == 4)
        return 0;
    if (ei_transition(r, EI_QUEUE))
        return -1;
    q->slots[(q->head + q->count) % 4] = r;
    ++q->count;
    q->bytes += 8;
    return 1;
}

int ei_dequeue(struct ei_queue *q, struct ei_request **r)
{
    if (!r || !queue_valid(q))
        return -1;
    if (!q->count)
        return 0;
    struct ei_request *owned = q->slots[q->head];
    if (ei_transition(owned, EI_ACCEPT))
        return -1;
    q->slots[q->head] = 0;
    q->head = (q->head + 1) % 4;
    --q->count;
    q->bytes -= 8;
    *r = owned;
    return 1;
}

void ei_reply(const unsigned char in[8], unsigned char out[4])
{
    uint32_t sum = 0;
    for (unsigned i = 0; i < 8; ++i)
        sum += in[i];
    out[0] = (unsigned char)(sum >> 24);
    out[1] = (unsigned char)(sum >> 16);
    out[2] = (unsigned char)(sum >> 8);
    out[3] = (unsigned char)sum;
}
