#include "semaphore.h"
#ifndef CDSEM_MUTANT
#define CDSEM_MUTANT 0
#endif
int cds_init(struct cd_sem *s, cmx_kernel *k, unsigned initial,
             unsigned maximum, unsigned priority)
{
    if (!s || !k || !maximum || initial > maximum || priority > 1)
        return CMX_INVALID;
    cw_init(&s->wait, k);
    s->initial = initial;
    s->priority_order = priority;
    s->closed = 0;
    return cw_object_init(&s->wait, 0, CW_SEMAPHORE, initial, maximum);
}

int cds_take(struct cd_sem *s, unsigned t, cmx_deadline d, struct cw_token *out)
{
    if (!s)
        return CMX_INVALID;
    if (s->closed)
        return CMX_CLOSED;
    return cw_begin(&s->wait, 0, t, d, 0, out);
}

int cds_post(struct cd_sem *s, struct cd_post *out)
{
    if (!s || !out)
        return CMX_INVALID;
    if (s->closed)
        return CDSEM_MUTANT == 5 ? CMX_OK : CMX_CLOSED;
    cmx_kernel *k = s->wait.kernel;
    cmx_sem *sem = &s->wait.objects[0].semaphore;
    unsigned fifo = CMX_TASKS, best = CMX_TASKS;
    struct cd_post result = {CMX_TASKS, 0, 0, 0};
    for (unsigned t = 0; t < CMX_TASKS; ++t)
    {
        cmx_task *p = &k->task[t];
        if (p->state != CMX_WAITING || p->wait != CMX_W_SEM || p->object != sem)
            continue;
        ++result.eligible;
        if (fifo == CMX_TASKS || p->wait_sequence < k->task[fifo].wait_sequence)
            fifo = t;
        if (best == CMX_TASKS || p->priority < k->task[best].priority ||
            (p->priority == k->task[best].priority &&
             p->wait_sequence < k->task[best].wait_sequence))
            best = t;
    }
    if (!s->priority_order || CDSEM_MUTANT == 2)
        best = fifo;
    /* The policy adapter temporarily exchanges only admission ordering stamps.
     * Full external exclusion prevents an observer seeing this projection.
     * Restore both original stamps after CMX's actual completion transition. */
    uint32_t first_sequence = 0, best_sequence = 0;
    if (best < CMX_TASKS)
    {
        first_sequence = k->task[fifo].wait_sequence;
        best_sequence = k->task[best].wait_sequence;
        k->task[fifo].wait_sequence = best_sequence;
        k->task[best].wait_sequence = first_sequence;
    }
    int status;
    if (CDSEM_MUTANT == 3 && sem->count == sem->maximum)
    {
        sem->count = 0;
        status = CMX_OK;
    }
    else
        status = cw_post(&s->wait, 0);
    if (best < CMX_TASKS)
    {
        k->task[fifo].wait_sequence = first_sequence;
        k->task[best].wait_sequence = best_sequence;
    }
    if (status != CMX_OK)
        return status;
    if (best < CMX_TASKS)
    {
        if (CDSEM_MUTANT == 1)
            ++sem->count;
        result.woke = 1;
        result.defer_schedule =
            k->current < 0 ||
            k->task[best].priority < k->task[k->current].priority;
    }
    if (CDSEM_MUTANT == 4)
        (void)cmx_schedule(k);
    *out = result;
    return CMX_OK;
}

int cds_close(struct cd_sem *s)
{
    if (!s)
        return CMX_INVALID;
    if (s->closed)
        return CMX_CLOSED;
    int r = cw_close(&s->wait, 0);
    if (r == CMX_OK)
        s->closed = 1;
    return r;
}

int cds_deferred_init(struct cd_deferred *d, unsigned maximum)
{
    if (!d || !maximum)
        return CMX_INVALID;
    *d = (struct cd_deferred){0};
    d->maximum = maximum;
    return CMX_OK;
}

int cds_deferred_offer(struct cd_deferred *d)
{
    if (!d)
        return CMX_INVALID;
    if (d->closed)
        return CMX_CLOSED;
    if (d->pending == d->maximum || d->accepted == UINT64_MAX)
        return CMX_FULL;
    ++d->pending;
    ++d->accepted;
    return CMX_OK;
}

int cds_deferred_drain(struct cd_deferred *d, struct cd_sem *s, unsigned budget,
                       unsigned *out)
{
    if (!d || !s || !out || !budget)
        return CMX_INVALID;
    if (d->closed)
        return CMX_CLOSED;
    unsigned done = 0;
    int status = CMX_OK;
    while (done < budget && d->pending)
    {
        struct cd_post post;
        status = cds_post(s, &post);
        if (status != CMX_OK)
            break;
        --d->pending;
        ++d->drained;
        ++done;
    }
    *out = done;
    return status;
}

int cds_deferred_close(struct cd_deferred *d)
{
    if (!d)
        return CMX_INVALID;
    if (d->closed)
        return CMX_CLOSED;
    if (d->cancelled > UINT64_MAX - d->pending)
        return CMX_FULL;
    d->cancelled += d->pending;
    d->pending = 0;
    d->closed = 1;
    return CMX_OK;
}
