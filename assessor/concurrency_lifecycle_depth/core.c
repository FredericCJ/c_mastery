#include "cl.h"
#ifndef CL_MUTANT
#define CL_MUTANT 0
#endif
void cl_queue_init(struct cl_queue *q, struct cl_message storage[2])
{
    q->slots = storage;
    q->head = 0;
    q->count = 0;
    q->closed = 0;
    q->accepted = 0;
    q->delivered = 0;
    q->discarded = 0;
}

enum cl_result cl_send(struct cl_queue *q, uint32_t p, uint32_t seq,
                       const unsigned char *in, size_t n)
{
    if (n > 4 || (n && !in))
    {
        return CL_RANGE;
    }
    if (q->closed)
    {
        return CL_CLOSED;
    }
    if (q->count == 2 && CL_MUTANT != 7)
    {
        return CL_FULL;
    }
    if (q->accepted == UINT64_MAX)
    {
        return CL_RANGE;
    }
    struct cl_message *s = &q->slots[(q->head + q->count) % 2];
    s->producer = p;
    s->sequence = seq;
    s->length = n;
    for (size_t i = 0; i < n; i++)
    {
        s->bytes[i] = CL_MUTANT == 1 ? 0 : in[i];
    }
    q->count++;
    q->accepted++;
    return CL_OK;
}

enum cl_result cl_recv(struct cl_queue *q, unsigned char *out, size_t cap,
                       size_t *n, uint32_t *p, uint32_t *seq)
{
    if (!n || !p || !seq)
    {
        return CL_RANGE;
    }
    if (!q->count)
    {
        return q->closed ? CL_CLOSED : CL_EMPTY;
    }
    struct cl_message *s = &q->slots[q->head];
    if (cap < s->length || (s->length && !out))
    {
        return CL_RANGE;
    }
    for (size_t i = 0; i < s->length; i++)
    {
        out[i] = s->bytes[i];
    }
    *n = s->length;
    *p = s->producer;
    *seq = s->sequence;
    if (CL_MUTANT != 2)
    {
        q->head = (q->head + 1) % 2;
        q->count--;
    }
    q->delivered++;
    return CL_OK;
}

void cl_close(struct cl_queue *q)
{
    q->closed = 1;
}

uint64_t cl_abort(struct cl_queue *q)
{
    unsigned n = q->count;
    q->discarded += n;
    q->count = 0;
    q->head = 0;
    q->closed = 1;
    return n;
}

static struct cl_allocation *allocation(struct cl_pool *p, struct cl_buffer h)
{
    if (h.slot >= 4 || p->slots[h.slot].generation != h.generation ||
        p->slots[h.slot].owner == CL_FREE)
    {
        return 0;
    }
    return &p->slots[h.slot];
}

void cl_pool_init(struct cl_pool *p)
{
    for (unsigned i = 0; i < 4; i++)
    {
        p->slots[i] = (struct cl_allocation){0, 0, 1, CL_FREE};
    }
    p->allocations = 0;
    p->releases = 0;
}

enum cl_result cl_alloc(struct cl_pool *p, unsigned n, struct cl_buffer *out)
{
    if (!out || n > 8)
    {
        return CL_RANGE;
    }
    if (p->allocations == UINT64_MAX)
    {
        return CL_RANGE;
    }
    uint32_t used = 0;
    unsigned free_slot = 4, free_bytes = 0;
    for (unsigned i = 0; i < 4; i++)
    {
        struct cl_allocation *a = &p->slots[i];
        if (a->owner == CL_FREE)
        {
            if (a->generation != UINT64_MAX && free_slot == 4)
            {
                free_slot = i;
            }
        }
        else
        {
            used |= ((UINT32_C(1) << a->length) - 1u) << a->offset;
        }
    }
    if (free_slot == 4)
    {
        return CL_FULL;
    }
    for (unsigned i = 0; i < 16; i++)
    {
        if (!(used & (UINT32_C(1) << i)))
        {
            free_bytes++;
        }
    }
    if (free_bytes < n)
    {
        return CL_FULL;
    }
    unsigned offset = 16;
    for (unsigned i = 0; i + n <= 16; i++)
    {
        uint32_t mask = ((UINT32_C(1) << n) - 1u) << i;
        if (!(used & mask))
        {
            offset = i;
            break;
        }
    }
    if (offset == 16 && n)
    {
        return CL_MUTANT == 12 ? CL_FULL : CL_FRAGMENTED;
    }
    struct cl_allocation *a = &p->slots[free_slot];
    a->offset = offset;
    a->length = n;
    a->owner = CL_CALLER;
    for (unsigned i = 0; i < n; i++)
    {
        p->bytes[offset + i] = 0;
    }
    *out = (struct cl_buffer){free_slot, a->generation};
    p->allocations++;
    return CL_OK;
}

enum cl_result cl_buffer_write(struct cl_pool *p, struct cl_buffer h,
                               const unsigned char *in, unsigned n)
{
    struct cl_allocation *a = allocation(p, h);
    if (!a)
    {
        return CL_STALE;
    }
    if (a->owner != CL_CALLER)
    {
        return CL_BUSY;
    }
    if (n != a->length || (n && !in))
    {
        return CL_RANGE;
    }
    for (unsigned i = 0; i < n; i++)
    {
        p->bytes[a->offset + i] = in[i];
    }
    return CL_OK;
}

enum cl_result cl_buffer_read(const struct cl_pool *p, struct cl_buffer h,
                              unsigned char *out, unsigned cap, unsigned *n)
{
    if (h.slot >= 4 || p->slots[h.slot].generation != h.generation ||
        p->slots[h.slot].owner == CL_FREE)
    {
        return CL_STALE;
    }
    const struct cl_allocation *a = &p->slots[h.slot];
    if (a->owner != CL_CALLER && a->owner != CL_CONSUMER)
    {
        return CL_BUSY;
    }
    if (!n || cap < a->length || (a->length && !out))
    {
        return CL_RANGE;
    }
    for (unsigned i = 0; i < a->length; i++)
    {
        out[i] = p->bytes[a->offset + i];
    }
    *n = a->length;
    return CL_OK;
}

enum cl_result cl_release(struct cl_pool *p, struct cl_buffer h)
{
    struct cl_allocation *a = allocation(p, h);
    if (!a)
    {
        return CL_STALE;
    }
    if (a->owner == CL_QUEUED && CL_MUTANT != 3)
    {
        return CL_BUSY;
    }
    a->owner = CL_FREE;
    a->generation++;
    p->releases++;
    return CL_OK;
}

void cl_owned_init(struct cl_owned_queue *q)
{
    q->head = 0;
    q->count = 0;
    q->closed = 0;
}

enum cl_result cl_owned_send(struct cl_pool *p, struct cl_owned_queue *q,
                             struct cl_buffer h)
{
    struct cl_allocation *a = allocation(p, h);
    if (!a)
    {
        return CL_STALE;
    }
    if (a->owner != CL_CALLER)
    {
        return CL_BUSY;
    }
    if (q->closed)
    {
        return CL_CLOSED;
    }
    if (q->count == 2)
    {
        return CL_FULL;
    }
    q->slots[(q->head + q->count) % 2] = h;
    q->count++;
    a->owner = CL_QUEUED;
    return CL_OK;
}

enum cl_result cl_owned_recv(struct cl_pool *p, struct cl_owned_queue *q,
                             struct cl_buffer *out)
{
    if (!out)
    {
        return CL_RANGE;
    }
    if (!q->count)
    {
        return q->closed ? CL_CLOSED : CL_EMPTY;
    }
    struct cl_buffer h = q->slots[q->head];
    struct cl_allocation *a = allocation(p, h);
    if (!a || a->owner != CL_QUEUED)
    {
        return CL_STALE;
    }
    a->owner = CL_CONSUMER;
    *out = h;
    q->head = (q->head + 1) % 2;
    q->count--;
    return CL_OK;
}

void cl_owned_close(struct cl_owned_queue *q)
{
    q->closed = 1;
}

unsigned cl_owned_abort(struct cl_pool *p, struct cl_owned_queue *q)
{
    unsigned n = q->count;
    while (q->count)
    {
        struct cl_buffer h;
        if (cl_owned_recv(p, q, &h) != CL_OK)
        {
            break;
        }
        (void)cl_release(p, h);
    }
    q->closed = 1;
    return n - q->count;
}

static struct cl_lease_slot *lease(struct cl_life *l, struct cl_lease h)
{
    if (l->generation != h.object_generation || h.slot >= 8 ||
        l->leases[h.slot].generation != h.generation ||
        l->leases[h.slot].state == CL_UNUSED)
    {
        return 0;
    }
    return &l->leases[h.slot];
}

void cl_life_init(struct cl_life *l)
{
    l->generation = 1;
    l->phase = CL_OPEN;
    for (unsigned i = 0; i < 8; i++)
    {
        l->leases[i] = (struct cl_lease_slot){1, CL_UNUSED};
    }
}

enum cl_result cl_acquire(struct cl_life *l, struct cl_lease *out)
{
    if (!out)
    {
        return CL_RANGE;
    }
    if (l->phase != CL_OPEN)
    {
        return CL_CLOSED;
    }
    for (unsigned i = 0; i < 8; i++)
    {
        if (l->leases[i].state == CL_UNUSED &&
            l->leases[i].generation != UINT64_MAX)
        {
            l->leases[i].state = CL_ACTIVE;
            *out = (struct cl_lease){l->generation, l->leases[i].generation, i};
            return CL_OK;
        }
    }
    return CL_FULL;
}

enum cl_result cl_transition(struct cl_life *l, struct cl_lease h,
                             enum cl_lease_state from, enum cl_lease_state to)
{
    struct cl_lease_slot *s = lease(l, h);
    if (!s)
    {
        return CL_STALE;
    }
    if (s->state != from)
    {
        return CL_BUSY;
    }
    if (from < CL_ACTIVE || from > CL_WAKE_CLOSED || to < CL_ACTIVE ||
        to > CL_WAKE_CLOSED)
    {
        return CL_RANGE;
    }
    if (l->phase != CL_OPEN && to != CL_ACTIVE && to != CL_WAKE_CLOSED)
    {
        return CL_CLOSED;
    }
    s->state = to;
    return CL_OK;
}

enum cl_result cl_drop(struct cl_life *l, struct cl_lease h)
{
    struct cl_lease_slot *s = lease(l, h);
    if (!s)
    {
        return CL_STALE;
    }
    if (s->state != CL_ACTIVE && s->state != CL_WAKE_CLOSED)
    {
        return CL_BUSY;
    }
    s->state = CL_UNUSED;
    s->generation++;
    return CL_OK;
}

void cl_life_close(struct cl_life *l)
{
    if (l->phase != CL_OPEN)
    {
        return;
    }
    l->phase = CL_CLOSING;
    for (unsigned i = 0; i < 8; i++)
    {
        if (l->leases[i].state == CL_SEND_WAIT ||
            l->leases[i].state == CL_RECV_WAIT ||
            (CL_MUTANT == 9 && l->leases[i].state == CL_COND_RELOCK))
        {
            l->leases[i].state = CL_WAKE_CLOSED;
        }
    }
}

enum cl_result cl_life_drained(struct cl_life *l)
{
    if (l->phase != CL_CLOSING)
    {
        return CL_CLOSED;
    }
    l->phase = CL_SHUT;
    return CL_OK;
}

enum cl_result cl_destroy(struct cl_life *l)
{
    if (l->phase != CL_SHUT)
    {
        return CL_BUSY;
    }
    for (unsigned i = 0; i < 8; i++)
    {
        if (l->leases[i].state != CL_UNUSED && CL_MUTANT != 4)
        {
            return CL_BUSY;
        }
    }
    l->phase = CL_DESTROYED;
    return CL_OK;
}

enum cl_result cl_restart(struct cl_life *l)
{
    if (l->phase != CL_DESTROYED)
    {
        return CL_BUSY;
    }
    if (l->generation == UINT64_MAX)
    {
        return CL_RANGE;
    }
    l->generation++;
    l->phase = CL_OPEN;
    for (unsigned i = 0; i < 8; i++)
    {
        l->leases[i] = (struct cl_lease_slot){1, CL_UNUSED};
    }
    return CL_OK;
}

enum cl_result cl_restart_group(struct cl_life *objects, unsigned count)
{
    if (!objects || !count || count > 3)
    {
        return CL_RANGE;
    }
    for (unsigned i = 0; i < count; i++)
    {
        if (objects[i].phase != CL_DESTROYED)
        {
            return CL_BUSY;
        }
        if (objects[i].generation == UINT64_MAX)
        {
            return CL_RANGE;
        }
    }
    /* Dependency order supplied as consumer first: restart providers first. */
    for (unsigned i = count; i > 0; i--)
    {
        (void)cl_restart(&objects[i - 1]);
    }
    return CL_OK;
}

void cl_record_init(struct cl_record *r, uint32_t v)
{
    *r = (struct cl_record){v, ~v, v, 0, 0, CL_HEALTHY};
}

enum cl_result cl_record_begin(struct cl_record *r, uint32_t owner)
{
    if (!owner)
    {
        return CL_RANGE;
    }
    if (r->state == CL_BROKEN)
    {
        return CL_UNRECOVERABLE;
    }
    if (r->state == CL_NEEDS_REPAIR || r->state == CL_REPAIRING)
    {
        return CL_POISONED;
    }
    if (r->state != CL_HEALTHY)
    {
        return CL_BUSY;
    }
    r->owner = owner;
    r->state = CL_OWNED;
    return CL_OK;
}

enum cl_result cl_record_stage(struct cl_record *r, uint32_t owner, uint32_t v)
{
    if (r->owner != owner || r->state != CL_OWNED)
    {
        return CL_BUSY;
    }
    r->value = v;
    r->state = CL_DIRTY;
    return CL_OK;
}

enum cl_result cl_record_commit(struct cl_record *r, uint32_t owner)
{
    if (r->owner != owner || (r->state != CL_OWNED && r->state != CL_DIRTY))
    {
        return CL_BUSY;
    }
    r->checksum = ~r->value;
    r->committed = r->value;
    r->owner = 0;
    r->repairs = 0;
    r->state = CL_HEALTHY;
    return CL_OK;
}

enum cl_result cl_record_dead(struct cl_record *r, uint32_t dead)
{
    if (!dead || r->owner != dead ||
        (r->state != CL_OWNED && r->state != CL_DIRTY &&
         r->state != CL_REPAIRING))
    {
        return CL_BUSY;
    }
    r->owner = 0;
    r->state = CL_NEEDS_REPAIR;
    return CL_OK;
}

enum cl_result cl_repair_begin(struct cl_record *r, uint32_t owner)
{
    if (!owner)
    {
        return CL_RANGE;
    }
    if (r->state == CL_BROKEN)
    {
        return CL_UNRECOVERABLE;
    }
    if (r->state != CL_NEEDS_REPAIR)
    {
        return CL_BUSY;
    }
    if (r->repairs == 2)
    {
        r->state = CL_BROKEN;
        return CL_UNRECOVERABLE;
    }
    r->repairs++;
    r->owner = owner;
    r->state = CL_REPAIRING;
    return CL_OK;
}

enum cl_result cl_repair_finish(struct cl_record *r, uint32_t owner,
                                unsigned repair)
{
    if (r->owner != owner || r->state != CL_REPAIRING)
    {
        return CL_BUSY;
    }
    r->owner = 0;
    if (!repair)
    {
        r->state = CL_BROKEN;
        return CL_UNRECOVERABLE;
    }
    r->value = r->committed + (CL_MUTANT == 5 ? 1u : 0u);
    r->checksum = ~r->value;
    r->state = CL_HEALTHY;
    return CL_OK;
}

enum cl_result cl_record_read(const struct cl_record *r, uint32_t *v)
{
    if (!v)
    {
        return CL_RANGE;
    }
    if (r->state == CL_BROKEN)
    {
        return CL_UNRECOVERABLE;
    }
    if (CL_MUTANT != 10 && (r->state != CL_HEALTHY || r->checksum != ~r->value))
    {
        return CL_POISONED;
    }
    *v = r->value;
    return CL_OK;
}

void cl_ack_init(struct cl_ack *a, uint32_t epoch)
{
    a->epoch = epoch;
    a->value = 0;
    a->highwater = 0;
    for (unsigned i = 0; i < 4; i++)
    {
        a->receipts[i] = (struct cl_receipt){0, 0, 0, 0};
    }
}

enum cl_result cl_request(struct cl_ack *a, uint32_t epoch, uint32_t id,
                          uint32_t delta, uint32_t *out)
{
    if (!id || !epoch || !out)
    {
        return CL_RANGE;
    }
    if (epoch != a->epoch)
    {
        return CL_STALE;
    }
    unsigned free_slot = 4;
    for (unsigned i = 0; i < 4; i++)
    {
        struct cl_receipt *r = &a->receipts[i];
        if (!r->state)
        {
            if (free_slot == 4)
            {
                free_slot = i;
            }
            continue;
        }
        if (r->id == id)
        {
            if (r->delta != delta && CL_MUTANT != 11)
            {
                return CL_RANGE;
            }
            if (r->state == 1)
            {
                return CL_PENDING;
            }
            if (r->state == 3)
            {
                return CL_RANGE;
            }
            if (CL_MUTANT == 6)
            {
                a->value += delta;
            }
            *out = r->result;
            return CL_OK;
        }
    }
    if (id <= a->highwater)
    {
        return CL_STALE;
    }
    if (free_slot == 4)
    {
        return CL_FULL;
    }
    a->receipts[free_slot] = (struct cl_receipt){1, id, delta, 0};
    a->highwater = id;
    return CL_PENDING;
}

enum cl_result cl_apply(struct cl_ack *a, uint32_t id)
{
    for (unsigned i = 0; i < 4; i++)
    {
        struct cl_receipt *r = &a->receipts[i];
        if (!r->state || r->id != id)
        {
            continue;
        }
        if (r->state == 2)
        {
            return CL_OK;
        }
        if (r->state == 3)
        {
            return CL_RANGE;
        }
        if (r->delta > UINT32_MAX - a->value)
        {
            r->state = 3;
            return CL_RANGE;
        }
        a->value += r->delta;
        r->result = a->value;
        r->state = 2;
        return CL_OK;
    }
    return CL_STALE;
}

enum cl_result cl_acknowledge(struct cl_ack *a, uint32_t id)
{
    for (unsigned i = 0; i < 4; i++)
    {
        if (a->receipts[i].state && a->receipts[i].id == id)
        {
            if (a->receipts[i].state == 1)
            {
                return CL_PENDING;
            }
            a->receipts[i].state = 0;
            return CL_OK;
        }
    }
    return CL_STALE;
}
