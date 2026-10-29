#include "dispatch_deferred.h"
#ifndef DS_DEFERRED_MUTANT
#define DS_DEFERRED_MUTANT 0
#endif
static struct ds_deferred *owner(struct ds_deferred_token token)
{
    struct ds_deferred *c = token.owner;
    return c && c->initialized && c->active && c->generation == token.generation
               ? c
               : NULL;
}

static void lock(struct ds_deferred *c)
{
    (void)pthread_mutex_lock(&c->mutex);
}

static void unlock(struct ds_deferred *c)
{
    (void)pthread_mutex_unlock(&c->mutex);
}

int ds_deferred_init(struct ds_deferred *c)
{
    if (!c)
        return DS_INVALID;
    *c = (struct ds_deferred){0};
    if (pthread_mutex_init(&c->mutex, NULL))
        return DS_FAILED;
    if (pthread_cond_init(&c->condition, NULL))
    {
        (void)pthread_mutex_destroy(&c->mutex);
        return DS_FAILED;
    }
    c->initialized = 1;
    c->generation = 1;
    return DS_OK;
}

int ds_deferred_destroy(struct ds_deferred *c)
{
    if (!c || !c->initialized)
        return DS_INVALID;
    if (c->active || c->job)
        return DS_BUSY;
    if (pthread_cond_destroy(&c->condition) || pthread_mutex_destroy(&c->mutex))
        return DS_FAILED;
    c->initialized = 0;
    return DS_OK;
}

int ds_deferred_bind(struct ds_deferred *c, struct ow_shared *environment,
                     ds_deferred_body_fn body, struct ds_deferred_token *out)
{
    if (!c || !c->initialized || !environment || !body || !out)
        return DS_INVALID;
    if (c->active)
        return DS_BUSY;
    if (c->generation > 3)
        return DS_FULL;
    if (DS_DEFERRED_MUTANT != 1 && ow_shared_retain(environment) != OW_OK)
        return DS_FAILED;
    c->environment = environment;
    c->body = body;
    c->active = 1;
    c->cancel = 0;
    *out = (struct ds_deferred_token){c, c->generation};
    return DS_OK;
}

static void *worker(void *context)
{
    struct ds_deferred *c = context;
    lock(c);
    while (!c->go && !c->cancel)
        (void)pthread_cond_wait(&c->condition, &c->mutex);
    if (c->cancel)
    {
        c->status = DS_STALE;
        c->done = 1;
        (void)pthread_cond_broadcast(&c->condition);
        unlock(c);
        return NULL;
    }
    c->running = 1;
    ds_deferred_body_fn body = c->body;
    void *environment = c->environment->context;
    int arg = c->argument;
    unlock(c);
    int out = 0;
    int status = body(environment, arg, &out);
    lock(c);
    c->result = out;
    c->status = status;
    c->running = 0;
    c->done = 1;
    (void)pthread_cond_broadcast(&c->condition);
    unlock(c);
    return NULL;
}

int ds_deferred_submit(struct ds_deferred_token token, int arg)
{
    struct ds_deferred *c = owner(token);
    if (!c)
        return DS_STALE;
    lock(c);
    if (c->job || c->cancel)
    {
        unlock(c);
        return DS_BUSY;
    }
    c->argument = arg;
    c->go = 0;
    c->done = 0;
    c->job = 1;
    if (pthread_create(&c->thread, NULL, worker, c))
    {
        c->job = 0;
        unlock(c);
        return DS_FAILED;
    }
    unlock(c);
    return DS_OK;
}

int ds_deferred_start(struct ds_deferred_token token)
{
    struct ds_deferred *c = owner(token);
    if (!c)
        return DS_STALE;
    lock(c);
    if (!c->job || c->go || c->cancel)
    {
        unlock(c);
        return DS_BUSY;
    }
    c->go = 1;
    (void)pthread_cond_broadcast(&c->condition);
    unlock(c);
    return DS_OK;
}

static struct ow_shared *consume(struct ds_deferred *c)
{
    struct ow_shared *environment = c->environment;
    c->environment = NULL;
    c->body = NULL;
    c->active = 0;
    c->job = 0;
    c->done = 0;
    c->go = 0;
    ++c->generation;
    return environment;
}

int ds_deferred_cancel(struct ds_deferred_token token)
{
    struct ds_deferred *c = owner(token);
    if (!c)
        return DS_STALE;
    lock(c);
    c->cancel = 1;
    (void)pthread_cond_broadcast(&c->condition);
    if (c->job)
    {
        unlock(c);
        return DS_BUSY;
    }
    struct ow_shared *environment = consume(c);
    unlock(c);
    return ow_shared_drop(environment) == OW_OK ? DS_OK : DS_FAILED;
}

int ds_deferred_drain(struct ds_deferred_token token, int *out)
{
    if (!out)
        return DS_INVALID;
    struct ds_deferred *c = owner(token);
    if (!c)
        return DS_STALE;
    lock(c);
    if (!c->job)
    {
        unlock(c);
        return DS_EMPTY;
    }
    if (!c->done)
    {
        unlock(c);
        return DS_BUSY;
    }
    unlock(c);
    if (pthread_join(c->thread, NULL))
        return DS_FAILED;
    lock(c);
    int rc = c->cancel && DS_DEFERRED_MUTANT != 2 ? DS_STALE : c->status;
    if (rc == DS_OK)
        *out = c->result;
    struct ow_shared *environment = consume(c);
    unlock(c);
    (void)ow_shared_drop(environment);
    return rc;
}
