#include "amp.h"
#include <limits.h>
#ifndef AF_MUTANT
#define AF_MUTANT 0
#endif
int af_elapsed(uint32_t start, uint32_t now, uint32_t interval,
               unsigned *expired)
{
    uint32_t delta = now - start;
    if (!expired || !interval || interval >= UINT32_C(0x80000000) ||
        delta >= UINT32_C(0x80000000))
    {
        return AF_INVALID;
    }
    *expired = delta >= interval;
    return AF_OK;
}

int af_boot_begin(struct af_boot_owner *s, uint32_t previous, uint32_t now,
                  uint32_t timeout, unsigned peer_quiet, unsigned dma_quiet)
{
    if (!s || !timeout || timeout >= UINT32_C(0x80000000))
    {
        return AF_INVALID;
    }
    if (s->active || !peer_quiet || !dma_quiet)
    {
        return AF_BUSY;
    }
    if (previous == UINT32_MAX)
    {
        return AF_EXHAUSTED;
    }
    *s = (struct af_boot_owner){previous + 1, 0, now, timeout, 1};
    return AF_OK;
}

int af_boot_stage(struct af_boot_owner *s, struct af_boot *b, unsigned stage,
                  uint32_t version, uint32_t layout)
{
    if (!s || !b || !s->active || stage > 3 || stage > s->stage || !version ||
        !layout)
    {
        return AF_INVALID;
    }
    if (stage < s->stage)
    {
        return AF_OK; /* Replaying a finished publication is inert. */
    }
    if (stage == 0)
    {
        atomic_store_explicit(&b->ready, 0, memory_order_release);
    }
    if (stage == 1)
    {
        atomic_store_explicit(&b->version, version, memory_order_relaxed);
        atomic_store_explicit(&b->layout, layout, memory_order_relaxed);
        atomic_store_explicit(&b->epoch, s->next_epoch, memory_order_relaxed);
    }
    if (stage == 2)
    {
        atomic_store_explicit(&b->ack, 0, memory_order_relaxed);
    }
    if (stage == 3)
    {
        atomic_store_explicit(&b->ready, s->next_epoch, memory_order_release);
    }
    s->stage++;
    return AF_OK;
}

int af_boot_ack(struct af_boot *b, uint32_t version, uint32_t layout,
                uint32_t epoch)
{
    if (!b || !version || !layout || !epoch)
    {
        return AF_INVALID;
    }
    uint32_t ready = atomic_load_explicit(&b->ready, memory_order_acquire);
    if (!ready)
    {
        return AF_BUSY;
    }
    if (ready != epoch ||
        atomic_load_explicit(&b->epoch, memory_order_relaxed) != epoch ||
        atomic_load_explicit(&b->version, memory_order_relaxed) != version ||
        (AF_MUTANT != 1 &&
         atomic_load_explicit(&b->layout, memory_order_relaxed) != layout))
    {
        return AF_STALE;
    }
    atomic_store_explicit(&b->ack, epoch, memory_order_release);
    return AF_OK;
}

int af_boot_status(const struct af_boot_owner *s, const struct af_boot *b,
                   uint32_t now)
{
    if (!s || !b || !s->active)
    {
        return AF_INVALID;
    }
    unsigned expired;
    int r = af_elapsed(s->start, now, s->timeout, &expired);
    if (r != AF_OK)
    {
        return r;
    }
    if (expired)
    {
        return AF_STALE;
    }
    if (s->stage == 4 &&
        atomic_load_explicit(&b->ack, memory_order_acquire) == s->next_epoch &&
        atomic_load_explicit(&b->ready, memory_order_acquire) == s->next_epoch)
    {
        return AF_OK;
    }
    return expired ? AF_STALE : AF_BUSY;
}

int af_boot_close(struct af_boot_owner *s, struct af_boot *b,
                  unsigned peer_quiet, unsigned dma_quiet)
{
    if (!s || !b)
    {
        return AF_INVALID;
    }
    atomic_store_explicit(&b->ready, 0, memory_order_release);
    if (!peer_quiet || !dma_quiet)
    {
        return AF_BUSY;
    }
    s->active = 0;
    return AF_OK;
}

void af_ring_init(struct af_ring *q, uint32_t initial)
{
    atomic_init(&q->head, initial);
    atomic_init(&q->tail, initial);
}

uint32_t af_queued(const struct af_ring *q)
{
    return atomic_load_explicit(&q->head, memory_order_acquire) -
           atomic_load_explicit(&q->tail, memory_order_acquire);
}

int af_batch(struct af_ring *q, uint32_t epoch, const struct af_message *v,
             unsigned n)
{
    if (!q || !v || !epoch || !n || n > 4)
    {
        return AF_INVALID;
    }
    for (unsigned i = 0; i < n; i++)
    {
        if (v[i].epoch != epoch || !v[i].sequence || v[i].length > AF_PAYLOAD)
        {
            return AF_INVALID;
        }
    }
    uint32_t head = atomic_load_explicit(&q->head, memory_order_relaxed),
             tail = atomic_load_explicit(&q->tail, memory_order_acquire),
             used = head - tail;
    if (used > AF_CAPACITY)
    {
        return AF_INVALID;
    }
    if (n > AF_CAPACITY - used)
    {
        return AF_FULL;
    }
    for (unsigned i = 0; i < n; i++)
    {
        q->slots[(head + i) & (AF_CAPACITY - 1)] = v[i];
    }
    atomic_store_explicit(&q->head, head + n, memory_order_release);
    return AF_OK;
}

int af_receive(struct af_ring *q, uint32_t epoch, void *out, size_t capacity,
               uint32_t *sequence, size_t *length)
{
    if (!q || !epoch || !out || !sequence || !length)
    {
        return AF_INVALID;
    }
    uint32_t tail = atomic_load_explicit(&q->tail, memory_order_relaxed),
             head = atomic_load_explicit(&q->head, memory_order_acquire),
             used = head - tail;
    if (!used)
    {
        return AF_EMPTY;
    }
    if (used > AF_CAPACITY)
    {
        return AF_INVALID;
    }
    const struct af_message *m = &q->slots[tail & (AF_CAPACITY - 1)];
    if (m->epoch != epoch)
    {
        return AF_STALE;
    }
    if (m->length > AF_PAYLOAD || m->length > capacity)
    {
        return AF_INVALID;
    }
    for (unsigned i = 0; i < m->length; i++)
    {
        ((unsigned char *)out)[i] = m->bytes[i];
    }
    *sequence = m->sequence;
    *length = m->length;
    atomic_store_explicit(&q->tail, tail + 1, memory_order_release);
    return AF_OK;
}

int af_cache_span(uint32_t address, uint32_t bytes, uint32_t line,
                  uint32_t *first, uint32_t *end)
{
    if (!first || !end || !bytes || !line || (line & (line - 1)) ||
        bytes > UINT32_MAX - address)
    {
        return AF_INVALID;
    }
    uint32_t high = address + bytes;
    if (high > UINT32_MAX - (line - 1))
    {
        return AF_INVALID;
    }
    *first = address & ~(line - 1);
    *end = (high + line - 1) & ~(line - 1);
    return AF_OK;
}

int af_effect(struct af_receipt *s, uint32_t session, uint32_t id,
              int32_t delta, int32_t *out)
{
    if (!s || !out || !session || !id)
    {
        return AF_INVALID;
    }
    if (session != s->session)
    {
        return AF_STALE;
    }
    if (id == s->last_id)
    {
        if (delta != s->last_delta)
        {
            return AF_CONFLICT;
        }
        *out = s->last_result;
        return AF_OK;
    }
    if (s->last_id == UINT32_MAX)
    {
        return AF_EXHAUSTED;
    }
    if (id != s->last_id + 1)
    {
        return AF_STALE;
    }
    if ((delta > 0 && s->total > INT32_MAX - delta) ||
        (delta < 0 && s->total < INT32_MIN - delta))
    {
        return AF_INVALID;
    }
    int32_t value = s->total + delta;
    s->total = value;
    s->last_id = id;
    s->last_delta = delta;
    s->last_result = value;
    *out = value;
    return AF_OK;
}

int af_region(const struct af_bank *banks, size_t count, uint32_t address,
              uint32_t bytes, unsigned masters, unsigned *domain)
{
    if (!banks || !domain || !bytes || !masters || bytes > UINT32_MAX - address)
    {
        return AF_INVALID;
    }
    for (size_t i = 0; i < count; i++)
    {
        if (address >= banks[i].base &&
            address - banks[i].base < banks[i].bytes &&
            bytes <= banks[i].bytes - (address - banks[i].base) &&
            (banks[i].masters & masters) == masters)
        {
            *domain = banks[i].domain;
            return AF_OK;
        }
    }
    return AF_INVALID;
}

int af_owner_claim(struct af_owners *o, unsigned resource, unsigned core)
{
    if (!o || resource > 3 || (core != 4 && core != 7))
    {
        return AF_INVALID;
    }
    unsigned *slot = resource == 0   ? &o->clock_owner
                     : resource == 1 ? &o->dma_owner
                     : resource == 2 ? &o->telemetry_owner
                                     : &o->compute_owner;
    if (*slot && *slot != core)
    {
        return AF_CONFLICT;
    }
    *slot = core;
    return AF_OK;
}
