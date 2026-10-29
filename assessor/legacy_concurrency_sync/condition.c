#include "condition.h"
#ifndef CDQ_MUTANT
#define CDQ_MUTANT 0
#endif
static int valid(const struct cd_queue *q, unsigned t)
{
    return q && !q->destroyed && t < CMX_TASKS &&
           q->kernel->task[t].state != CMX_UNUSED &&
           q->kernel->task[t].state != CMX_EXITED;
}

static int owns(const struct cd_queue *q, unsigned t)
{
    return valid(q, t) && q->mutex.owner == (int)t;
}

int cdq_init(struct cd_queue *q, cmx_kernel *k, unsigned capacity,
             unsigned broadcast)
{
    if (!q || !k || !capacity || capacity > 2 || broadcast > 1)
        return CMX_INVALID;
    /* Fresh storage only. Register the eventual embedded address before
     * publishing. */
    int result = cmx_mutex_init(k, &q->mutex, 0, 0);
    if (result != CMX_OK)
        return result;
    cmx_mutex mutex = q->mutex;
    *q = (struct cd_queue){0};
    q->kernel = k;
    q->mutex = mutex;
    q->capacity = capacity;
    q->broadcast = broadcast;
    return CMX_OK;
}

int cdq_pulse(struct cd_queue *q, cmx_mutex *held, unsigned t,
              unsigned broadcast)
{
    if (!valid(q, t) || broadcast > 1 || (CDQ_MUTANT != 5 && held != &q->mutex))
        return CMX_INVALID;
    if (held->owner != (int)t)
        return CMX_NOT_OWNER;
    return cmx_cond_signal(q->kernel, &q->changed, broadcast);
}

static void notify(struct cd_queue *q)
{
    (void)cmx_cond_signal(q->kernel, &q->changed,
                          CDQ_MUTANT == 3 ? 0 : q->broadcast);
}

int cdq_put_locked(struct cd_queue *q, unsigned t, uint32_t value)
{
    if (!valid(q, t))
        return CMX_INVALID;
    if (!owns(q, t))
        return CMX_NOT_OWNER;
    if (q->closed)
        return CMX_CLOSED;
    if (q->accepted == UINT64_MAX)
        return CMX_FULL;
    if (q->count == q->capacity)
        return CMX_TIMEOUT;
    q->values[(q->head + q->count) % q->capacity] = value;
    q->count++;
    q->accepted++;
    notify(q);
    return CMX_OK;
}

static uint32_t remove_item(struct cd_queue *q)
{
    uint32_t value = q->values[q->head];
    q->head = (q->head + 1) % q->capacity;
    q->count--;
    notify(q);
    return value;
}

int cdq_get_locked(struct cd_queue *q, unsigned t, uint32_t *value)
{
    if (!valid(q, t) || !value)
        return CMX_INVALID;
    if (!owns(q, t))
        return CMX_NOT_OWNER;
    if (!q->count)
        return q->closed ? CMX_CLOSED : CMX_TIMEOUT;
    *value = remove_item(q);
    q->consumed++;
    return CMX_OK;
}

static int finish(struct cd_queue *q, unsigned t, int result)
{
    q->request[t].phase = CDQ_DONE;
    q->request[t].result = result;
    if (q->mutex.owner == (int)t)
        (void)cmx_mutex_unlock(q->kernel, &q->mutex, t);
    return result;
}

static int predicate(struct cd_queue *q, unsigned t, int awakened)
{
    struct cdq_request *r = &q->request[t];
    if (r->operation == CDQ_GET)
    {
        if (q->count || (CDQ_MUTANT == 1 && awakened))
        {
            r->value = remove_item(q);
            q->claimed++;
            return finish(q, t, CMX_OK);
        }
        if (q->closed)
            return finish(q, t, CMX_CLOSED);
    }
    else
    {
        int result = cdq_put_locked(q, t, r->value);
        if (result != CMX_TIMEOUT)
            return finish(q, t, result);
    }
    int result =
        cmx_cond_wait(q->kernel, &q->changed, &q->mutex, t, r->deadline);
    if (result == CMX_BLOCKED)
    {
        r->phase = CDQ_CONDITION;
        return result;
    }
    return finish(q, t, result);
}

int cdq_begin(struct cd_queue *q, unsigned t, unsigned operation,
              uint32_t value, cmx_deadline deadline)
{
    if (!valid(q, t) || operation > CDQ_GET ||
        q->request[t].phase != CDQ_IDLE ||
        (q->kernel->task[t].state != CMX_READY &&
         q->kernel->task[t].state != CMX_RUNNING))
        return CMX_INVALID;
    if (q->mutex.owner == (int)t)
        return CMX_DEADLOCK;
    q->request[t] =
        (struct cdq_request){CDQ_LOCK, operation, value, deadline, CMX_BLOCKED};
    int result = cmx_mutex_lock(q->kernel, &q->mutex, t, deadline);
    if (result == CMX_BLOCKED)
        return result;
    if (result != CMX_OK)
        return finish(q, t, result);
    return predicate(q, t, 0);
}

int cdq_begin_locked(struct cd_queue *q, unsigned t, unsigned operation,
                     uint32_t value, cmx_deadline deadline)
{
    if (!valid(q, t) || operation > CDQ_GET || q->request[t].phase != CDQ_IDLE)
        return CMX_INVALID;
    if (!owns(q, t))
        return CMX_NOT_OWNER;
    q->request[t] =
        (struct cdq_request){CDQ_LOCK, operation, value, deadline, CMX_BLOCKED};
    return predicate(q, t, 0);
}

int cdq_resume(struct cd_queue *q, unsigned t)
{
    if (!valid(q, t) || q->request[t].phase == CDQ_IDLE)
        return CMX_INVALID;
    struct cdq_request *r = &q->request[t];
    cmx_task *task = &q->kernel->task[t];
    if (r->phase == CDQ_DONE)
        return r->result;
    if (task->state == CMX_WAITING)
    {
        if (CDQ_MUTANT == 2 && r->phase == CDQ_CONDITION &&
            task->wait == CMX_W_MUTEX && task->pending != CMX_OK)
        {
            r->phase = CDQ_DONE;
            r->result = task->pending;
            return r->result;
        }
        return CMX_BLOCKED;
    }
    if (task->result != CMX_OK)
        return finish(q, t, task->result);
    if (q->mutex.owner != (int)t)
        return CMX_BROKEN;
    return predicate(q, t, r->phase == CDQ_CONDITION);
}

int cdq_result(struct cd_queue *q, unsigned t, int *result, uint32_t *value)
{
    if (!valid(q, t) || !result || !value || q->request[t].phase == CDQ_IDLE)
        return CMX_INVALID;
    struct cdq_request *r = &q->request[t];
    if (r->phase != CDQ_DONE)
        return CMX_BLOCKED;
    *result = r->result;
    if (r->operation == CDQ_GET && r->result == CMX_OK)
    {
        *value = r->value;
        q->claimed--;
        q->consumed++;
    }
    *r = (struct cdq_request){0};
    return CMX_OK;
}

int cdq_shutdown(struct cd_queue *q, unsigned t)
{
    if (!valid(q, t))
        return CMX_INVALID;
    if (!owns(q, t))
        return CMX_NOT_OWNER;
    q->closed = 1;
    if (CDQ_MUTANT != 4)
        (void)cmx_cond_close(q->kernel, &q->changed);
    return CMX_OK;
}

int cdq_destroy(struct cd_queue *q)
{
    if (!q || q->destroyed)
        return CMX_INVALID;
    if (q->count || q->claimed)
        return CMX_BLOCKED;
    for (unsigned t = 0; t < CMX_TASKS; t++)
        if (q->request[t].phase != CDQ_IDLE)
            return CMX_BLOCKED;
    int result = cmx_mutex_close(q->kernel, &q->mutex);
    if (result == CMX_OK)
        q->destroyed = 1;
    return result;
}
