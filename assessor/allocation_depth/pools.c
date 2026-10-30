#include "allocation.h"

/*
 * The algorithms-family pool has 16 slots; this wrapper exposes the first
 * three and marks the others permanently retired (state 2), so the pool is
 * small enough to exhaust in a test. A slot is also retired once it has been
 * used for its third generation.
 */
enum
{
    POOL_USABLE_SLOTS = 3,
    POOL_PRODUCER_SLOTS = 16,
    POOL_RETIRED_STATE = 2,
    POOL_LAST_GENERATION = 3
};

void al_pool_init(struct al_pool *p)
{
    algo_pool_init(&p->producer);
    for (unsigned i = POOL_USABLE_SLOTS; i < POOL_PRODUCER_SLOTS; ++i)
    {
        p->producer.state[i] = POOL_RETIRED_STATE;
    }
}

int al_pool_take(struct al_pool *p, int32_t value, struct al_pool_handle *out)
{
    if (!p || !out)
    {
        return AL_BAD;
    }
    uint64_t raw;
    int status = algo_pool_take(&p->producer, value, &raw);
    if (status == ALGO_FULL)
    {
        return AL_FULL;
    }
    if (status != ALGO_OK)
    {
        return AL_BAD;
    }
    *out = (struct al_pool_handle){p, raw};
    return AL_OK;
}

int al_pool_get(const struct al_pool *p, struct al_pool_handle h, int32_t *out)
{
    if (!p || !out)
    {
        return AL_BAD;
    }
#if AL_MUTANT != 6
    if (h.owner != p)
    {
        return AL_STALE;
    }
#endif
    return algo_pool_get(&p->producer, h.raw, out) == ALGO_OK ? AL_OK
                                                              : AL_STALE;
}

/* The low 32 bits of the raw handle are the slot index. */
int al_pool_release(struct al_pool *p, struct al_pool_handle h)
{
    if (!p)
    {
        return AL_BAD;
    }
    if (h.owner != p)
    {
        return AL_STALE;
    }
    unsigned slot = (unsigned)h.raw;
    int32_t value;
    if (algo_pool_get(&p->producer, h.raw, &value) != ALGO_OK)
    {
        return AL_STALE;
    }
    int retire = p->producer.generation[slot] >= POOL_LAST_GENERATION;
    int status = algo_pool_release(&p->producer, h.raw);
    if (retire)
    {
        p->producer.state[slot] = POOL_RETIRED_STATE;
    }
    return status == ALGO_OK ? AL_OK : AL_STALE;
}

int al_classes_init(struct al_classes *c)
{
    if (!c)
    {
        return AL_BAD;
    }
    *c = (struct al_classes){0};
    return AL_OK;
}

/* Class index for a request of 1..32 bytes: 0 holds <=8, 1 <=16, 2 <=32. */
static unsigned class_of(size_t n)
{
    if (n <= 8)
    {
        return 0;
    }
    return n <= 16 ? 1 : 2;
}

/* Bytes charged per slot of class `group`: 8, 16 or 32. */
static unsigned class_bytes(unsigned group)
{
    return 8u << group;
}

/*
 * The initializer fills a scratch buffer first, so a failing initializer
 * (AL_CLEANUP) leaves the table untouched; the callback is responsible for
 * undoing its own partial effects.
 */
int al_classes_take(struct al_classes *c, size_t n,
                    int (*init)(void *, unsigned char *, size_t),
                    al_cleanup_fn destroy, void *ctx, struct al_handle *out)
{
    if (!c || !n || n > AL_CLASS_BYTES || !out || !init)
    {
        return AL_BAD;
    }
    if (c->serial == UINT32_MAX)
    {
        return AL_RETIRED;
    }
    unsigned group = class_of(n);
    for (unsigned i = 0; i < AL_CLASS_SLOTS; ++i)
    {
        struct al_class_slot *slot = &c->slots[group][i];
        if (slot->live)
        {
            continue;
        }
        unsigned char data[AL_CLASS_BYTES] = {0};
        if (init(ctx, data, n))
        {
            return AL_CLEANUP;
        }
        for (size_t j = 0; j < n; ++j)
        {
            slot->data[j] = data[j];
        }
        slot->id = ++c->serial;
        slot->live = 1;
        slot->requested = (unsigned)n;
        slot->cleanup = destroy;
        slot->context = ctx;
        c->retained[group] = 1;
        *out = (struct al_handle){c, slot->id};
        return AL_OK;
    }
    return AL_FULL;
}

/* The slot is freed and zeroed even when its cleanup fails (AL_CLEANUP). */
int al_classes_release(struct al_classes *c, struct al_handle h)
{
    if (!c)
    {
        return AL_BAD;
    }
    if (h.owner != c || !h.id)
    {
        return AL_STALE;
    }
    for (unsigned g = 0; g < AL_CLASS_GROUPS; ++g)
    {
        for (unsigned i = 0; i < AL_CLASS_SLOTS; ++i)
        {
            struct al_class_slot *slot = &c->slots[g][i];
            if (!slot->live || slot->id != h.id)
            {
                continue;
            }
            slot->live = 0;
            int status = slot->cleanup ? slot->cleanup(slot->context) : 0;
            for (unsigned k = 0; k < AL_CLASS_BYTES; ++k)
            {
                slot->data[k] = 0;
            }
            return status ? AL_CLEANUP : AL_OK;
        }
    }
    return AL_STALE;
}

int al_classes_stats(const struct al_classes *c, size_t *payload,
                     size_t *charged, size_t *retained)
{
    if (!c || !payload || !charged || !retained)
    {
        return AL_BAD;
    }
    size_t requested = 0;
    size_t charge = 0;
    size_t reserved = 0;
    for (unsigned g = 0; g < AL_CLASS_GROUPS; ++g)
    {
        unsigned size = class_bytes(g);
        if (c->retained[g])
        {
            reserved += (size_t)AL_CLASS_SLOTS * size;
        }
        for (unsigned i = 0; i < AL_CLASS_SLOTS; ++i)
        {
            if (c->slots[g][i].live)
            {
                requested += c->slots[g][i].requested;
                charge += size;
            }
        }
    }
    *payload = requested;
    *charged = charge;
    *retained = reserved;
    return AL_OK;
}
