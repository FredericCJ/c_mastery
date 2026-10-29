#include "pressure.h"
#include <limits.h>

static bool value_ok(const struct rzp_value *v)
{
    return v && v->owned && v->dispose;
}

void rzp_value_drop(struct rzp_value *v)
{
    if (v && v->owned)
    {
        void *owned = v->owned;
        void (*dispose)(void *) = v->dispose;
        *v = (struct rzp_value){0};
        dispose(owned);
    }
}

void rzp_owned_init(struct rzp_owned_queue *q)
{
    if (q)
    {
        *q = (struct rzp_owned_queue){0};
    }
}

int rzp_owned_offer(struct rzp_owned_queue *q, const struct rz_scope *scope,
                    uint64_t now, enum rz_policy policy,
                    struct rzp_value *value)
{
    if (!q || !scope || !value_ok(value) || policy < RZ_BLOCK ||
        policy > RZ_DEGRADE)
    {
        return RZ_INVALID;
    }
    size_t head = q->core.head, count = q->core.count, last = (head + 3) % 4;
    struct rz_offer offer = {scope, value->value, true};
    int status = rz_offer(&q->core, &offer, policy, now);
    if (status == RZ_PENDING)
    {
        return status;
    }
    if (status == RZ_OK)
    {
        size_t index = (head + count) % 4;
        if (count == 4)
        {
#if RZF_MUTANT != 53
            rzp_value_drop(&q->values[index]);
#endif
        }
        q->values[index] = *value;
        *value = (struct rzp_value){0};
    }
    else if (status == RZ_DEGRADED && value->value > q->values[last].value)
    {
        rzp_value_drop(&q->values[last]);
        q->values[last] = *value;
        *value = (struct rzp_value){0};
    }
    else
    {
        rzp_value_drop(value);
    }
    return status;
}

int rzp_owned_pop(struct rzp_owned_queue *q, struct rzp_value *out)
{
    if (!q || !out)
    {
        return RZ_INVALID;
    }
    size_t index = q->core.head;
    unsigned unused;
    int status = rz_pop(&q->core, &unused);
    if (status == RZ_OK)
    {
        *out = q->values[index];
        q->values[index] = (struct rzp_value){0};
    }
    return status;
}

void rzp_owned_close(struct rzp_owned_queue *q)
{
    if (q)
    {
        q->core.closed = true;
    }
}

enum
{
    FREE,
    QUEUED,
    TRANSPORT,
    LEASED,
    RETIRED
};

static void finish(struct rzp_producer *p, int result)
{
    rzp_value_drop(&p->value);
    p->pending = false;
    p->result = result;
    (void)rzf_borrow_ack(&p->scope);
}

static void progress(struct rzp_pump *p)
{
    unsigned start = p->next;
    for (unsigned j = 0; j < 2; ++j)
    {
        unsigned k = (start + j) % 2;
        struct rzp_producer *producer = &p->producer[k];
        if (!producer->active || !producer->pending)
        {
            continue;
        }
        int status = rz_check(&producer->scope.scope->core, p->last);
        if (status != RZ_OK)
        {
            finish(producer, status);
            continue;
        }
        if (p->closed)
        {
            finish(producer, RZ_CLOSED);
            continue;
        }
        if (p->consumed == UINT64_MAX)
        {
            finish(producer, RZ_FULL);
            continue;
        }
        if (!p->credits)
        {
            continue;
        }
        unsigned slot = 0;
        while (slot < 4 && p->slots[slot].state != FREE)
        {
            ++slot;
        }
        if (slot == 4)
        {
            continue;
        }
        struct rzp_slot *s = &p->slots[slot];
        struct rz_offer offer = {&producer->scope.scope->core, slot, true};
        status = rz_offer(&p->queue, &offer, RZ_BLOCK, p->last);
        if (status != RZ_OK)
        {
            continue;
        }
        ++s->generation;
        s->state = QUEUED;
        s->value = producer->value;
        producer->value = (struct rzp_value){0};
        --p->credits;
        ++p->consumed;
        p->next = (k + 1) % 2;
        finish(producer, RZ_OK);
    }
    pthread_cond_broadcast(&p->changed);
}

static bool ticket_ok(const struct rzp_pump *p, struct rzp_ticket t)
{
    return t.owner == p && t.index < 2 && p->producer[t.index].active &&
           p->producer[t.index].generation == t.generation;
}

int rzp_init(struct rzp_pump *p, size_t capacity)
{
    if (!p || !capacity || capacity > 4)
    {
        return RZ_INVALID;
    }
    *p = (struct rzp_pump){0};
    if (pthread_mutex_init(&p->mutex, NULL))
    {
        return RZ_CLOSED;
    }
    if (pthread_cond_init(&p->changed, NULL))
    {
        pthread_mutex_destroy(&p->mutex);
        return RZ_CLOSED;
    }
    if (msg_hub_init(&p->hub, capacity, 1) != MSG_OK)
    {
        pthread_cond_destroy(&p->changed);
        pthread_mutex_destroy(&p->mutex);
        return RZ_CLOSED;
    }
    p->credits = 4;
    p->initialized = true;
    return RZ_OK;
}

int rzp_begin(struct rzp_pump *p, unsigned producer, struct rzf_scope *scope,
              uint64_t now, struct rzp_value *value, struct rzp_ticket *out)
{
    if (!p || !p->initialized || producer > 1 || !scope || !value_ok(value) ||
        !out)
    {
        return RZ_INVALID;
    }
    pthread_mutex_lock(&p->mutex);
    int status = RZ_OK;
    struct rzp_producer *slot = &p->producer[producer];
    if (now < p->last)
    {
        status = RZ_RANGE;
    }
    else if (p->closed)
    {
        status = RZ_CLOSED;
    }
    else if (slot->active)
    {
        status = RZ_BUSY;
    }
    else if (slot->generation == UINT_MAX)
    {
        status = RZ_FULL;
    }
#if RZF_MUTANT == 51
    else if (!p->credits)
    {
        status = RZ_PENDING;
    }
#endif
    else if ((status = rzf_borrow_begin(&slot->scope, scope)) == RZ_OK)
    {
        /* Register the bounded owner before checking the readiness predicate. */
        slot->active = slot->pending = true;
        ++slot->generation;
        slot->value = *value;
        *value = (struct rzp_value){0};
        p->last = now;
        *out = (struct rzp_ticket){p, producer, slot->generation};
        progress(p);
        status = slot->pending ? RZ_PENDING : slot->result;
    }
    pthread_mutex_unlock(&p->mutex);
    return status;
}

int rzp_poll(struct rzp_pump *p, struct rzp_ticket ticket, bool wait, int *out)
{
    if (!p || !p->initialized || !out)
    {
        return RZ_INVALID;
    }
    pthread_mutex_lock(&p->mutex);
    int status = RZ_STALE;
    if (ticket_ok(p, ticket))
    {
        struct rzp_producer *slot = &p->producer[ticket.index];
        if (slot->polling)
        {
            status = RZ_BUSY;
        }
        else
        {
            slot->polling = true;
            while (slot->pending && wait)
            {
                pthread_cond_wait(&p->changed, &p->mutex);
            }
            if (slot->pending)
            {
                status = RZ_PENDING;
            }
            else
            {
                *out = slot->result;
                slot->active = false;
                status = RZ_OK;
            }
            slot->polling = false;
        }
    }
    pthread_mutex_unlock(&p->mutex);
    return status;
}

int rzp_cancel(struct rzp_pump *p, struct rzp_ticket ticket)
{
    if (!p || !p->initialized)
    {
        return RZ_INVALID;
    }
    pthread_mutex_lock(&p->mutex);
    int status = RZ_STALE;
    if (ticket_ok(p, ticket))
    {
        struct rzp_producer *slot = &p->producer[ticket.index];
        status = RZ_DONE;
        if (slot->pending)
        {
            finish(slot, RZ_CANCELLED);
            pthread_cond_broadcast(&p->changed);
            status = RZ_OK;
        }
    }
    pthread_mutex_unlock(&p->mutex);
    return status;
}

int rzp_tick(struct rzp_pump *p, uint64_t now)
{
    if (!p || !p->initialized)
    {
        return RZ_INVALID;
    }
    pthread_mutex_lock(&p->mutex);
    int status = RZ_RANGE;
    if (now >= p->last)
    {
        p->last = now;
        progress(p);
        status = RZ_OK;
    }
    pthread_mutex_unlock(&p->mutex);
    return status;
}

int rzp_forward(struct rzp_pump *p)
{
    if (!p || !p->initialized)
    {
        return RZ_INVALID;
    }
    pthread_mutex_lock(&p->mutex);
    unsigned index = p->queue.count ? p->queue.values[p->queue.head] : 0;
    int status = rz_forward(&p->queue, &p->hub, 0);
    if (status == RZ_OK)
    {
        p->slots[index].state = TRANSPORT;
    }
    if (p->closed && !p->queue.count)
    {
        (void)msg_close(&p->hub, 0);
    }
    pthread_mutex_unlock(&p->mutex);
    return status;
}

int rzp_receive(struct rzp_pump *p, struct rzp_value *out,
                struct rzp_credit *credit)
{
    if (!p || !p->initialized || !out || !credit)
    {
        return RZ_INVALID;
    }
    pthread_mutex_lock(&p->mutex);
    unsigned channel, index;
    int status = RZ_INVALID;
    enum msg_status result = msg_select(&p->hub, 1, false, &channel, &index);
    if (result == MSG_OK)
    {
        struct rzp_slot *slot = &p->slots[index];
        slot->state = LEASED;
        *out = slot->value;
        *credit = (struct rzp_credit){p, index, slot->generation};
        status = RZ_OK;
#if RZF_MUTANT == 54
        ++p->credits;
        ++p->returned;
#endif
    }
    else if (result == MSG_EMPTY)
    {
        status = RZ_EMPTY;
    }
    else if (result == MSG_CLOSED)
    {
        status = RZ_CLOSED;
    }
    pthread_mutex_unlock(&p->mutex);
    return status;
}

int rzp_release(struct rzp_pump *p, struct rzp_credit credit)
{
    if (!p || !p->initialized)
    {
        return RZ_INVALID;
    }
    pthread_mutex_lock(&p->mutex);
    int status = RZ_STALE;
    if (credit.owner == p && credit.index < 4)
    {
        struct rzp_slot *slot = &p->slots[credit.index];
#if RZF_MUTANT == 52
        if (slot->generation == credit.generation && slot->state == FREE)
        {
            ++p->credits;
            ++p->returned;
            status = RZ_OK;
        }
#endif
        if (slot->generation == credit.generation && slot->state == LEASED)
        {
            rzp_value_drop(&slot->value);
            slot->state = slot->generation == UINT_MAX ? RETIRED : FREE;
            if (slot->state == FREE)
            {
                ++p->credits;
                ++p->returned;
            }
            progress(p);
            status = RZ_OK;
        }
    }
    pthread_mutex_unlock(&p->mutex);
    return status;
}

int rzp_close(struct rzp_pump *p)
{
    if (!p || !p->initialized)
    {
        return RZ_INVALID;
    }
    pthread_mutex_lock(&p->mutex);
    p->closed = true;
    progress(p);
    if (!p->queue.count)
    {
        (void)msg_close(&p->hub, 0);
    }
    pthread_mutex_unlock(&p->mutex);
    return RZ_OK;
}

int rzp_snapshot(struct rzp_pump *p, struct rzp_snapshot *out)
{
    if (!p || !p->initialized || !out)
    {
        return RZ_INVALID;
    }
    pthread_mutex_lock(&p->mutex);
    struct rzp_snapshot s = {0};
    for (unsigned i = 0; i < 4; ++i)
    {
        s.queued += p->slots[i].state == QUEUED;
        s.transport += p->slots[i].state == TRANSPORT;
        s.leased += p->slots[i].state == LEASED;
        s.retired += p->slots[i].state == RETIRED;
    }
    for (unsigned i = 0; i < 2; ++i)
    {
        s.pending += p->producer[i].active && p->producer[i].pending;
        s.terminal += p->producer[i].active && !p->producer[i].pending;
    }
    s.credits = p->credits;
    s.consumed = p->consumed;
    s.returned = p->returned;
    *out = s;
    pthread_mutex_unlock(&p->mutex);
    return RZ_OK;
}

int rzp_destroy(struct rzp_pump *p)
{
    if (!p || !p->initialized)
    {
        return RZ_INVALID;
    }
    /* Caller has joined all threads; no concurrent entry is permitted here. */
    if (!p->closed || p->queue.count)
    {
        return RZ_BUSY;
    }
    for (unsigned i = 0; i < 2; ++i)
    {
        if (p->producer[i].active)
        {
            return RZ_BUSY;
        }
    }
    for (unsigned i = 0; i < 4; ++i)
    {
        if (p->slots[i].state != FREE && p->slots[i].state != RETIRED)
        {
            return RZ_BUSY;
        }
    }
    msg_hub_destroy(&p->hub);
    pthread_cond_destroy(&p->changed);
    pthread_mutex_destroy(&p->mutex);
    p->initialized = false;
    return RZ_OK;
}
