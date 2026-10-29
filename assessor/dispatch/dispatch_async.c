#include "dispatch_async.h"
#include <limits.h>
#ifndef DS_ASYNC_MUTANT
#define DS_ASYNC_MUTANT 0
#endif
static struct ds_async_entry *entry(struct ds_async_token token)
{
    if (!token.registry || token.slot >= 4)
        return NULL;
    struct ds_async_entry *e = &token.registry->entries[token.slot];
    return e->live && e->generation == token.generation ? e : NULL;
}

static void lock(struct ds_async_registry *r)
{
    (void)pthread_mutex_lock(&r->mutex);
}

static void unlock(struct ds_async_registry *r)
{
    (void)pthread_mutex_unlock(&r->mutex);
}

int ds_async_init(struct ds_async_registry *r)
{
    if (!r)
        return DS_INVALID;
    *r = (struct ds_async_registry){0};
    if (pthread_mutex_init(&r->mutex, NULL))
        return DS_FAILED;
    if (pthread_cond_init(&r->condition, NULL))
    {
        (void)pthread_mutex_destroy(&r->mutex);
        return DS_FAILED;
    }
    for (unsigned i = 0; i < 4; ++i)
    {
        r->entries[i].registry = r;
        r->entries[i].generation = 1;
    }
    r->initialized = 1;
    return DS_OK;
}

int ds_async_destroy(struct ds_async_registry *r)
{
    if (!r || !r->initialized)
        return DS_INVALID;
    for (unsigned i = 0; i < 4; ++i)
        if (r->entries[i].live || r->entries[i].job)
            return DS_BUSY;
    if (pthread_cond_destroy(&r->condition))
        return DS_FAILED;
    if (pthread_mutex_destroy(&r->mutex))
        return DS_FAILED;
    r->initialized = 0;
    return DS_OK;
}

int ds_async_register(struct ds_async_registry *r, struct ds_loaded *owner,
                      struct ds_async_token *out)
{
    if (!r || !r->initialized || !owner || !out)
        return DS_INVALID;
    if (r->attempts == UINT_MAX)
        return DS_FULL;
    ++r->attempts;
    if (r->attempts == r->fail_at)
        return DS_FAILED;
    unsigned i = 0;
    while (i < 4 && (r->entries[i].live || r->entries[i].generation > 3))
        ++i;
    if (i == 4)
        return DS_FULL;
    const struct ds_plugin *api = NULL;
    int rc = ds_loaded_acquire(owner, &api);
    if (rc != DS_OK)
        return rc;
    struct ds_async_entry *e = &r->entries[i];
    unsigned generation = e->generation;
    *e = (struct ds_async_entry){.registry = r,
                                 .owner = owner,
                                 .api = api,
                                 .generation = generation,
                                 .live = 1,
                                 .registration_held = 1};
    ++r->registrations;
    *out = (struct ds_async_token){r, i, generation};
    return DS_OK;
}

static void *worker(void *context)
{
    struct ds_async_entry *e = context;
    struct ds_async_registry *r = e->registry;
    lock(r);
    while (!e->go && !e->cancel)
        (void)pthread_cond_wait(&r->condition, &r->mutex);
    if (e->cancel)
    {
        e->status = DS_STALE;
        e->done = 1;
        (void)pthread_cond_broadcast(&r->condition);
        unlock(r);
        return NULL;
    }
    e->running = 1;
    ++r->invocations;
    int state = e->state, amount = e->amount;
    const struct ds_plugin *api = e->api;
    unlock(r);
    int result = 0;
    int status = api->apply(state, amount, &result);
    lock(r);
    e->result = result;
    e->status = status;
    e->running = 0;
    e->done = 1;
    (void)pthread_cond_broadcast(&r->condition);
    unlock(r);
    return NULL;
}

int ds_async_queue(struct ds_async_token token, int state, int amount)
{
    struct ds_async_entry *e = entry(token);
    if (!e)
        return DS_STALE;
    struct ds_async_registry *r = token.registry;
    lock(r);
    if (e->job || e->cancel)
    {
        unlock(r);
        return DS_BUSY;
    }
    const struct ds_plugin *api = NULL;
    int rc = ds_loaded_acquire(e->owner, &api);
    if (rc != DS_OK)
    {
        unlock(r);
        return rc;
    }
    e->api = api;
    e->state = state;
    e->amount = amount;
    e->job = 1;
    e->go = 0;
    e->done = 0;
    e->running = 0;
    if (pthread_create(&e->thread, NULL, worker, e))
    {
        e->job = 0;
        (void)ds_loaded_release(e->owner);
        unlock(r);
        return DS_FAILED;
    }
    unlock(r);
    return DS_OK;
}

int ds_async_start(struct ds_async_token token)
{
    struct ds_async_entry *e = entry(token);
    if (!e)
        return DS_STALE;
    struct ds_async_registry *r = token.registry;
    lock(r);
    if (!e->job || e->go || e->cancel)
    {
        unlock(r);
        return DS_BUSY;
    }
    e->go = 1;
    (void)pthread_cond_broadcast(&r->condition);
    unlock(r);
    return DS_OK;
}

int ds_async_remove(struct ds_async_token token)
{
    struct ds_async_entry *e = entry(token);
    if (!e)
        return DS_STALE;
    struct ds_async_registry *r = token.registry;
    lock(r);
    e->cancel = 1;
    (void)pthread_cond_broadcast(&r->condition);
    if (e->job)
    {
        if (DS_ASYNC_MUTANT == 1 && e->registration_held)
        {
            (void)ds_loaded_release(e->owner);
            e->registration_held = 0;
        }
        unlock(r);
        return DS_BUSY;
    }
    if (e->registration_held)
        (void)ds_loaded_release(e->owner);
    e->registration_held = 0;
    e->live = 0;
    ++e->generation;
    ++r->acknowledgements;
    unlock(r);
    return DS_OK;
}

int ds_async_drain(struct ds_async_token token, int *out)
{
    struct ds_async_entry *e = entry(token);
    if (!e || !out)
        return !out ? DS_INVALID : DS_STALE;
    struct ds_async_registry *r = token.registry;
    lock(r);
    if (!e->job)
    {
        unlock(r);
        return DS_EMPTY;
    }
    if (!e->done)
    {
        unlock(r);
        return DS_BUSY;
    }
    unlock(r);
    if (pthread_join(e->thread, NULL))
        return DS_FAILED;
    lock(r);
    int rc = e->cancel && DS_ASYNC_MUTANT != 2 ? DS_STALE : e->status;
    if (rc == DS_OK)
        *out = e->result;
    e->job = 0;
    e->done = 0;
    e->go = 0;
    (void)ds_loaded_release(e->owner);
    unlock(r);
    return rc;
}

_Static_assert(CHAR_BIT == 8 && INT_MAX == 2147483647,
               "32-bit state codec target");

static void encode(unsigned char *p, int32_t v)
{
    uint32_t u = (uint32_t)v;
    for (unsigned i = 0; i < 4; ++i)
        p[i] = (unsigned char)(u >> (8 * i));
}

static int32_t decode(const unsigned char *p)
{
    uint32_t u = 0;
    for (unsigned i = 0; i < 4; ++i)
        u |= (uint32_t)p[i] << (8 * i);
    return u <= INT32_MAX ? (int32_t)u : -1 - (int32_t)(UINT32_MAX - u);
}

void ds_reload_init(struct ds_reload *h, struct ds_async_registry *r,
                    struct d_allocator allocator)
{
    *h = (struct ds_reload){.registry = r, .current = 2, .retired = 2};
    for (unsigned i = 0; i < 2; ++i)
        d_small_init(&h->states[i], allocator);
}

static int cancel_slot(struct ds_reload *h, unsigned slot)
{
    int result = DS_OK;
    for (unsigned i = 0; i < 2; ++i)
        if (h->registrations[slot][i].registry)
        {
            int rc = ds_async_remove(h->registrations[slot][i]);
            if (rc == DS_OK)
                h->registrations[slot][i] = (struct ds_async_token){0};
            else
                result = rc;
        }
    return result;
}

static void rollback(struct ds_reload *h, unsigned slot)
{
    (void)cancel_slot(h, slot);
    d_small_reset(&h->states[slot]);
    if (ds_loaded_close(&h->modules[slot]) != DS_OK)
        h->retired = slot;
}

int ds_reload_change(struct ds_reload *h, const char *path,
                     const int32_t *initial, size_t count)
{
    if (!h || !path || !h->registry || (!initial && count) || count > 8 ||
        (h->current < 2 && (initial || count)))
        return DS_INVALID;
    if (h->retired < 2)
        return DS_BUSY;
    if (h->generation == 3)
        return DS_FULL;
    unsigned slot = h->current == 0 ? 1 : 0;
    int rc = ds_loaded_open(&h->modules[slot], path);
    if (rc != DS_OK)
        return rc;
    unsigned previous = h->current < 2 ? h->modules[h->current].api->schema : 0;
    if (h->current < 2)
        count = h->states[h->current].length / 4;
    unsigned char bytes[32] = {0};
    for (size_t i = 0; i < count; ++i)
    {
        int value = h->current < 2
                        ? decode(d_small_data(&h->states[h->current]) + 4 * i)
                        : initial[i];
        int migrated = 0;
        rc = h->modules[slot].api->migrate(previous, value, &migrated);
        if (rc != DS_OK)
        {
            rollback(h, slot);
            return rc;
        }
        encode(bytes + 4 * i, (int32_t)migrated);
    }
    if (d_small_append(&h->states[slot], bytes, 4 * count) != D_OK)
    {
        rollback(h, slot);
        return DS_FAILED;
    }
    for (unsigned i = 0; i < 2; ++i)
    {
        rc = ds_async_register(h->registry, &h->modules[slot],
                               &h->registrations[slot][i]);
        if (rc != DS_OK)
        {
            if (DS_ASYNC_MUTANT != 3)
                rollback(h, slot);
            return rc;
        }
    }
    unsigned old = h->current;
    h->current = slot;
    ++h->generation;
    if (old < 2)
    {
        h->retired = old;
        (void)cancel_slot(h, old);
    }
    return DS_OK;
}

int ds_reload_collect(struct ds_reload *h, ds_collect_gate_fn gate,
                      void *context)
{
    if (!h || !gate)
        return DS_INVALID;
    if (h->retired == 2)
        return DS_OK;
    unsigned slot = h->retired;
    int rc = cancel_slot(h, slot);
    if (rc != DS_OK)
        return rc;
    rc = gate(context);
    if (rc != DS_OK)
    {
        if (DS_ASYNC_MUTANT == 4)
            d_small_reset(&h->states[slot]);
        return rc;
    }
    rc = ds_loaded_close(&h->modules[slot]);
    if (rc != DS_OK)
        return rc;
    d_small_reset(&h->states[slot]);
    h->retired = 2;
    return DS_OK;
}

int ds_reload_stop(struct ds_reload *h, ds_collect_gate_fn gate, void *context)
{
    if (!h || !gate)
        return DS_INVALID;
    if (h->retired < 2)
        return DS_BUSY;
    if (h->current < 2)
    {
        h->retired = h->current;
        h->current = 2;
    }
    return ds_reload_collect(h, gate, context);
}

int ds_reload_get(const struct ds_reload *h, size_t index, int32_t *out)
{
    if (!h || !out)
        return DS_INVALID;
    if (h->current == 2)
        return DS_EMPTY;
    if (index >= h->states[h->current].length / 4)
        return DS_RANGE;
    *out = decode(d_small_data(&h->states[h->current]) + 4 * index);
    return DS_OK;
}
