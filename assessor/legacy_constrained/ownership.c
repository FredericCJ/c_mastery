#include "ownership.h"
#ifndef CR_MUTANT
#define CR_MUTANT 0
#endif
static int lookup(const struct cr_pool *p, struct cr_token t,
                  const struct cr_payload **slot)
{
    if (!p || t.slot >= 3)
    {
        return CR_INVALID;
    }
    if (t.world != p->world || t.generation != p->slots[t.slot].generation ||
        p->slots[t.slot].state == CR_FREE ||
        p->slots[t.slot].state == CR_RETIRED)
    {
        return CR_STALE;
    }
    *slot = p->slots + t.slot;
    return CR_OK;
}

int cr_pool_init(struct cr_pool *p, uint64_t world)
{
    if (!p || !world)
    {
        return CR_INVALID;
    }
    *p = (struct cr_pool){.world = world};
    return CR_OK;
}

int cr_allocate(struct cr_pool *p, int fault, const unsigned char *input,
                size_t length, struct cr_token *out)
{
    if (!p || !out || (!input && length) || length > 16 ||
        (fault != 0 && fault != 1))
    {
        return CR_INVALID;
    }
    if (p->closed)
    {
        return CR_BUSY;
    }
    unsigned begin = fault ? 2 : 0, end = fault || CR_MUTANT == 1 ? 3 : 2;
    for (unsigned i = begin; i < end; i++)
    {
        if (p->slots[i].state == CR_FREE)
        {
            struct cr_payload *s = p->slots + i;
            if (s->generation == UINT64_MAX)
            {
                return CR_RANGE;
            }
            for (size_t j = 0; j < length; j++)
            {
                s->bytes[j] = input[j];
            }
            s->length = length;
            s->generation++;
            s->state = CR_OWNED;
            *out = (struct cr_token){p->world, s->generation, i};
            return CR_OK;
        }
    }
    return CR_FULL;
}

static int transition(struct cr_pool *p, struct cr_token t, unsigned from,
                      unsigned to)
{
    const struct cr_payload *s;
    int status = lookup(p, t, &s);
    if (status)
    {
        return status;
    }
    if (s->state != from)
    {
        return CR_BUSY;
    }
    p->slots[t.slot].state = to;
    return CR_OK;
}

int cr_offer(struct cr_pool *p, struct cr_token t)
{
    return transition(p, t, CR_OWNED, CR_OFFERED);
}

int cr_abort_offer(struct cr_pool *p, struct cr_token t)
{
    return transition(p, t, CR_OFFERED, CR_OWNED);
}

int cr_claim(struct cr_pool *p, struct cr_token t)
{
    return transition(p, t, CR_OFFERED, CR_CONSUMER);
}

int cr_read(const struct cr_pool *p, struct cr_token t, unsigned char *out,
            size_t capacity, size_t *length)
{
    if (!out || !length)
    {
        return CR_INVALID;
    }
    const struct cr_payload *s;
    int status = lookup(p, t, &s);
    if (status)
    {
        return status;
    }
    if (s->state != CR_CONSUMER)
    {
        return CR_BUSY;
    }
    if (capacity < s->length)
    {
        return CR_FULL;
    }
    for (size_t i = 0; i < s->length; i++)
    {
        out[i] = s->bytes[i];
    }
    *length = s->length;
    return CR_OK;
}

int cr_release(struct cr_pool *p, struct cr_token t)
{
    const struct cr_payload *s;
    int status = lookup(p, t, &s);
    if (status)
    {
        return status;
    }
    if (s->state != CR_OWNED && s->state != CR_CONSUMER && CR_MUTANT != 2)
    {
        return CR_BUSY;
    }
    p->slots[t.slot].length = 0;
    p->slots[t.slot].state = s->generation == UINT64_MAX ? CR_RETIRED : CR_FREE;
    return CR_OK;
}

int cr_close(struct cr_pool *p)
{
    if (!p)
    {
        return CR_INVALID;
    }
    p->closed = 1;
    return CR_OK;
}

int cr_drained(const struct cr_pool *p)
{
    if (!p)
    {
        return CR_INVALID;
    }
    if (!p->closed)
    {
        return CR_BUSY;
    }
    for (unsigned i = 0; i < 3; i++)
    {
        if (p->slots[i].state != CR_FREE && p->slots[i].state != CR_RETIRED)
        {
            return CR_BUSY;
        }
    }
    return CR_OK;
}

int cr_budget(const uint64_t *parts, size_t count, int known,
              uint64_t available, uint64_t *out)
{
    if (!parts || !out || !count || count > 32)
    {
        return CR_INVALID;
    }
    if (!known && CR_MUTANT != 3)
    {
        return CR_UNKNOWN;
    }
    uint64_t sum = 0;
    for (size_t i = 0; i < count; i++)
    {
        if (UINT64_MAX - sum < parts[i])
        {
            return CR_RANGE;
        }
        sum += parts[i];
    }
    if (sum > available)
    {
        return CR_FULL;
    }
    *out = sum;
    return CR_OK;
}
