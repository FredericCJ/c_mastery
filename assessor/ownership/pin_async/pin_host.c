#include "pin_host.h"

static int owner(struct opa_port *p)
{
    return p && p->ready && pthread_equal(pthread_self(), p->owner);
}

static void *backend(void *arg)
{
    struct opa_port *p = arg;
    pthread_mutex_lock(&p->mutex);
    while (!p->gate)
    {
        pthread_cond_wait(&p->condition, &p->mutex);
    }
    int32_t *buffer = p->buffer, value = p->value;
    size_t count = p->count;
    opa_receiver_fn receiver = p->receiver;
    void *context = p->context;
    uint32_t ticket = p->ticket;
    pthread_mutex_unlock(&p->mutex);
    int32_t sum = 0;
    for (size_t i = 0; i < count; ++i)
    {
        buffer[i] = value + (int32_t)i;
        sum += buffer[i];
    }
    pthread_mutex_lock(&p->mutex);
    p->worker_status = opa_notify(&p->core, ticket, 9, sum);
    pthread_mutex_unlock(&p->mutex);
    receiver(context, sum);
    pthread_mutex_lock(&p->mutex);
    p->done = 1;
    pthread_cond_broadcast(&p->condition);
    pthread_mutex_unlock(&p->mutex);
    return NULL;
}

enum ow_status opa_port_init(struct opa_port *p, int32_t *buffer, size_t count)
{
    if (!p || !buffer || !count || count > 16)
    {
        return OW_INVALID;
    }
    if (pthread_mutex_init(&p->mutex, NULL) != 0)
    {
        return OW_INVALID;
    }
    if (pthread_cond_init(&p->condition, NULL) != 0)
    {
        pthread_mutex_destroy(&p->mutex);
        return OW_INVALID;
    }
    p->owner = pthread_self();
    p->buffer = buffer;
    p->count = count;
    p->receiver = NULL;
    p->context = NULL;
    p->ticket = 0;
    p->started = 0;
    p->gate = 0;
    p->done = 0;
    p->joined = 0;
    p->worker_status = OW_OK;
    p->value = 0;
    enum ow_status q = opa_init(&p->core, buffer, 7, 9);
    if (q != OW_OK)
    {
        pthread_cond_destroy(&p->condition);
        pthread_mutex_destroy(&p->mutex);
        return q;
    }
    p->ready = 1;
    return OW_OK;
}

enum ow_status opa_port_start(struct opa_port *p, int32_t value,
                              opa_receiver_fn receiver, void *context,
                              uint32_t *out)
{
    if (!owner(p) || !receiver || !out || value < -1000000 || value > 1000000)
    {
        return OW_INVALID;
    }
    pthread_mutex_lock(&p->mutex);
    if (p->started)
    {
        pthread_mutex_unlock(&p->mutex);
        return OW_BUSY;
    }
    struct opa_machine before = p->core;
    uint32_t ticket = 0;
    enum ow_status q = opa_submit(&p->core, p->buffer, 7, &ticket);
    if (q != OW_OK)
    {
        pthread_mutex_unlock(&p->mutex);
        return q;
    }
    p->receiver = receiver;
    p->context = context;
    p->value = value;
    p->ticket = ticket;
    p->started = 1;
    p->gate = 0;
    p->done = 0;
    p->joined = 0;
    p->worker_status = OW_BUSY;
    if (pthread_create(&p->worker, NULL, backend, p) != 0)
    {
        p->core = before;
        p->started = 0;
        p->receiver = NULL;
        p->context = NULL;
        pthread_mutex_unlock(&p->mutex);
        return OW_NOMEM;
    }
    *out = ticket;
    pthread_mutex_unlock(&p->mutex);
    return OW_OK;
}

enum ow_status opa_port_cancel(struct opa_port *p, uint32_t ticket, int timeout)
{
    if (!owner(p) || (timeout != 0 && timeout != 1))
    {
        return OW_INVALID;
    }
    pthread_mutex_lock(&p->mutex);
    enum ow_status q = timeout ? opa_timeout(&p->core, ticket, 7)
                               : opa_cancel(&p->core, ticket, 7);
    pthread_mutex_unlock(&p->mutex);
    return q;
}

enum ow_status opa_port_allow_backend(struct opa_port *p, uint32_t ticket)
{
    if (!owner(p))
    {
        return OW_INVALID;
    }
    pthread_mutex_lock(&p->mutex);
    enum ow_status q = OW_OK;
    if (ticket != p->ticket)
    {
        q = OW_STALE;
    }
    else if (!p->started)
    {
        q = OW_INVALID;
    }
    else
    {
        p->gate = 1;
        pthread_cond_broadcast(&p->condition);
    }
    pthread_mutex_unlock(&p->mutex);
    return q;
}

enum ow_status opa_port_observe(struct opa_port *p, struct opa_observation *out)
{
    if (!owner(p) || !out)
    {
        return OW_INVALID;
    }
    pthread_mutex_lock(&p->mutex);
    *out = (struct opa_observation){
        p->core.pin.pending,      p->core.recovery,
        p->core.notified,         p->done,
        p->core.async.generation, p->core.async.releases};
    pthread_mutex_unlock(&p->mutex);
    return OW_OK;
}

enum ow_status opa_port_join(struct opa_port *p, uint32_t ticket, int wait,
                             int32_t *out)
{
    if (!owner(p) || !out || (wait != 0 && wait != 1))
    {
        return OW_INVALID;
    }
    pthread_mutex_lock(&p->mutex);
    if (ticket != p->ticket)
    {
        pthread_mutex_unlock(&p->mutex);
        return OW_STALE;
    }
    if (!p->started)
    {
        pthread_mutex_unlock(&p->mutex);
        return OW_INVALID;
    }
    while (!p->done)
    {
        if (!wait)
        {
            pthread_mutex_unlock(&p->mutex);
            return OW_BUSY;
        }
        pthread_cond_wait(&p->condition, &p->mutex);
    }
    unsigned joined = p->joined;
    pthread_t thread = p->worker;
    pthread_mutex_unlock(&p->mutex);
    if (!joined && pthread_join(thread, NULL) != 0)
    {
        return OW_BUSY;
    }
    pthread_mutex_lock(&p->mutex);
    p->joined = 1;
    enum ow_status q = p->worker_status;
    if (q == OW_OK)
    {
        q = opa_finish(&p->core, ticket, 7, out);
    }
    if (q == OW_OK)
    {
        p->started = 0;
        p->receiver = NULL;
        p->context = NULL;
    }
    pthread_mutex_unlock(&p->mutex);
    return q;
}

enum ow_status opa_port_rebind(struct opa_port *p, int32_t *buffer,
                               size_t count)
{
    if (!owner(p) || !buffer || !count || count > 16)
    {
        return OW_INVALID;
    }
    pthread_mutex_lock(&p->mutex);
    enum ow_status q = OW_BUSY;
    if (!p->started)
    {
        q = opa_rebind(&p->core, buffer, 7);
        if (q == OW_OK)
        {
            p->buffer = buffer;
            p->count = count;
        }
    }
    pthread_mutex_unlock(&p->mutex);
    return q;
}

enum ow_status opa_port_destroy(struct opa_port *p)
{
    if (!owner(p))
    {
        return OW_INVALID;
    }
    pthread_mutex_lock(&p->mutex);
    if (p->started)
    {
        pthread_mutex_unlock(&p->mutex);
        return OW_BUSY;
    }
    pthread_mutex_unlock(&p->mutex);
    if (pthread_cond_destroy(&p->condition) != 0)
    {
        return OW_BUSY;
    }
    if (pthread_mutex_destroy(&p->mutex) != 0)
    {
        return OW_BUSY;
    }
    p->ready = 0;
    return OW_OK;
}
