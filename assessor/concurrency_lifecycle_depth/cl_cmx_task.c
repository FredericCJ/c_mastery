#include "cl_cmx_task.h"
#ifndef CC_TASK_MUTANT
#define CC_TASK_MUTANT 0
#endif
static int may_wait(struct cc_handle h)
{
    uint32_t ipsr, primask, basepri, faultmask;
    __asm__ volatile(
        "mrs %0,ipsr\nmrs %1,primask\nmrs %2,basepri\nmrs %3,faultmask"
        : "=r"(ipsr), "=r"(primask), "=r"(basepri), "=r"(faultmask));
    if (CC_TASK_MUTANT != 2 && (ipsr || primask || basepri || faultmask))
    {
        return 0;
    }
    return h.owner && h.owner->kernel == &cmx_machine && h.task < CMX_TASKS &&
           cmx_machine.current == (int)h.task &&
           cmx_machine.task[h.task].state == CMX_RUNNING;
}

static int complete(int result, struct cc_ticket ticket, struct cl_message *out,
                    uint32_t saved)
{
    if (result == CC_BLOCKED && CC_TASK_MUTANT != 1)
    {
        cmx_port_yield();
    }
    cmx_port_unlock(saved);
    if (result != CL_OK && result != CC_BLOCKED)
    {
        return result;
    }
    /* The retained PendSV boundary suspends a blocked stack before this point.
     * Reacquire exclusion because IRQ progress can run after dispatch. */
    saved = cmx_port_lock();
    result = cc_result(ticket, out);
    cmx_port_unlock(saved);
    return result;
}

int cc_task_send(struct cc_handle h, uint32_t producer, uint32_t sequence,
                 const unsigned char *bytes, size_t length,
                 cmx_deadline deadline)
{
    if (!may_wait(h))
    {
        return CC_INVALID;
    }
    uint32_t saved = cmx_port_lock();
    struct cc_ticket ticket = {0};
    int result =
        cc_send(h, producer, sequence, bytes, length, deadline, &ticket);
    return complete(result, ticket, 0, saved);
}

int cc_task_receive(struct cc_handle h, size_t capacity, cmx_deadline deadline,
                    struct cl_message *out)
{
    if (!may_wait(h) || !out)
    {
        return CC_INVALID;
    }
    uint32_t saved = cmx_port_lock();
    struct cc_ticket ticket = {0};
    int result = cc_receive(h, capacity, deadline, &ticket);
    return complete(result, ticket, out, saved);
}

int cc_task_condition(struct cc_handle h, cmx_cond *condition, cmx_mutex *mutex,
                      cmx_deadline deadline)
{
    if (!may_wait(h))
    {
        return CC_INVALID;
    }
    uint32_t saved = cmx_port_lock();
    struct cc_ticket ticket = {0};
    int result = cc_condition(h, condition, mutex, deadline, &ticket);
    return complete(result, ticket, 0, saved);
}

int cc_task_condition_unlock(struct cc_handle h)
{
    if (!may_wait(h))
    {
        return CC_INVALID;
    }
    uint32_t saved = cmx_port_lock();
    int result = cc_condition_unlock(h);
    cmx_port_unlock(saved);
    return result;
}

int cc_task_close(struct cc_queue *q, unsigned abort_queued)
{
    if (!q || q->kernel != &cmx_machine)
    {
        return CC_INVALID;
    }
    uint32_t saved = cmx_port_lock();
    int result = cc_close(q, abort_queued);
    cmx_port_unlock(saved);
    return result;
}
