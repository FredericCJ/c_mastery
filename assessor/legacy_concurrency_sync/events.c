#include "events.h"
#ifndef CDE_MUTANT
#define CDE_MUTANT 0
#endif
static int valid(struct cd_events *e)
{
    return e && e->kernel;
}

static int callable(struct cd_events *e, unsigned task)
{
    return valid(e) && task < CMX_TASKS &&
           (e->kernel->task[task].state == CMX_READY ||
            e->kernel->task[task].state == CMX_RUNNING);
}

int cde_init(struct cd_events *e, cmx_kernel *k)
{
    if (!e || !k)
        return CMX_INVALID;
    *e = (struct cd_events){0};
    e->kernel = k;
    return CMX_OK;
}

int cde_wait(struct cd_events *e, unsigned task, uint32_t mask, unsigned all,
             unsigned consume, cmx_deadline deadline)
{
    if (!callable(e, task) || !mask || all > 1 || consume > 1 ||
        e->operation[task].phase != CDE_IDLE)
        return CMX_INVALID;
    if (CDE_MUTANT == 1)
        all = 0;
    if (CDE_MUTANT == 3)
        consume = 1;
    if (CDE_MUTANT == 7)
        mask &= UINT32_C(0x7fffffff);
    int result = cmx_event_wait(e->kernel, &e->event, task, mask, all, consume,
                                deadline);
    struct cde_operation *op = &e->operation[task];
    op->phase = result == CMX_BLOCKED ? CDE_PENDING : CDE_DONE;
    op->result = result;
    if (result == CMX_OK)
    {
        op->observed = e->kernel->task[task].value;
        if (CDE_MUTANT == 2 && consume)
            e->event.bits = 0;
    }
    return result;
}

int cde_result(struct cd_events *e, unsigned task, uint32_t *observed)
{
    if (!valid(e) || task >= CMX_TASKS || !observed ||
        e->operation[task].phase == CDE_IDLE)
        return CMX_INVALID;
    struct cde_operation *op = &e->operation[task];
    if (op->phase == CDE_PENDING)
    {
        if (e->kernel->task[task].state == CMX_WAITING)
            return CMX_BLOCKED;
        op->result = e->kernel->task[task].result;
        op->observed = e->kernel->task[task].value;
    }
    int result = op->result;
    if (result == CMX_OK)
        *observed = op->observed;
    op->phase = CDE_IDLE;
    return result;
}

int cde_set(struct cd_events *e, uint32_t bits)
{
    if (!valid(e))
        return CMX_INVALID;
    if (e->event.closed)
        return CMX_CLOSED;
    uint32_t saved[CMX_TASKS];
    if (CDE_MUTANT == 4)
        for (unsigned i = 0; i < CMX_TASKS; i++)
        {
            saved[i] = e->kernel->task[i].wait_sequence;
            e->kernel->task[i].wait_sequence = UINT32_MAX - saved[i];
        }
    cmx_event_set(e->kernel, &e->event, bits);
    if (CDE_MUTANT == 4)
        for (unsigned i = 0; i < CMX_TASKS; i++)
            e->kernel->task[i].wait_sequence = saved[i];
    return CMX_OK;
}

int cde_clear(struct cd_events *e, uint32_t bits)
{
    if (!valid(e))
        return CMX_INVALID;
    if (e->event.closed)
        return CMX_CLOSED;
    e->event.bits &= ~bits;
    return CMX_OK;
}

int cde_close(struct cd_events *e)
{
    if (!valid(e))
        return CMX_INVALID;
    if (CDE_MUTANT == 8)
        e->event.closed = 1;
    else
        cmx_event_close(e->kernel, &e->event);
    return CMX_OK;
}

int cde_destroy(struct cd_events *e)
{
    if (!valid(e))
        return CMX_INVALID;
    for (unsigned i = 0; i < CMX_TASKS; i++)
        if (e->operation[i].phase != CDE_IDLE)
            return CMX_BLOCKED;
    e->kernel = 0;
    return CMX_OK;
}

void cde_sequence_init(struct cde_sequence *s)
{
    *s = (struct cde_sequence){{0, 0}};
}

int cde_sequence_publish(struct cde_sequence *s)
{
    if (!s)
        return CMX_INVALID;
    if (s->current.serial == UINT32_MAX)
    {
        if (s->current.epoch == UINT32_MAX)
            return CMX_FULL;
        if (CDE_MUTANT != 5)
            s->current.epoch++;
        s->current.serial = 0;
    }
    else
        s->current.serial++;
    return CMX_OK;
}

int cde_sequence_read(const struct cde_sequence *s, struct cde_cursor *cursor,
                      uint64_t *delta, uint64_t *missed)
{
    if (!s || !cursor || !delta || !missed)
        return CMX_INVALID;
    uint64_t now = ((uint64_t)s->current.epoch << 32) | s->current.serial;
    uint64_t old = ((uint64_t)cursor->epoch << 32) | cursor->serial;
    if (old > now)
        return CMX_INVALID;
    if (old == now)
        return CMX_BLOCKED;
    uint64_t change = CDE_MUTANT == 6 ? 1 : now - old;
    *delta = change;
    *missed = change - 1;
    *cursor = s->current;
    return CMX_OK;
}
