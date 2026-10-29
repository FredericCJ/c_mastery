#include "mutex.h"
#include <limits.h>
#ifndef CDM_MUTANT
#define CDM_MUTANT 0
#endif

static int live(const cmx_kernel *k, unsigned task)
{
    return k && task < CMX_TASKS &&
           (k->task[task].state == CMX_READY ||
            k->task[task].state == CMX_RUNNING);
}

static int registered(const cmx_kernel *k, const cmx_mutex *m)
{
    for (unsigned i = 0; i < k->mutex_count; ++i)
        if (k->mutex[i] == m)
            return 1;
    return 0;
}

int cdm_recursive_init(struct cd_recursive *r, cmx_kernel *k, unsigned maximum)
{
    if (!r || !k || !maximum)
        return CMX_INVALID;
    int result = cmx_mutex_init(k, &r->mutex, 0, 0);
    if (result != CMX_OK)
        return result;
    r->kernel = k;
    r->maximum = maximum;
    r->depth = 0;
    return CMX_OK;
}

int cdm_recursive_lock(struct cd_recursive *r, unsigned task, cmx_deadline d)
{
    if (!r || !live(r->kernel, task))
        return CMX_INVALID;
    if (r->mutex.closed)
        return CMX_CLOSED;
    if (r->mutex.owner == (int)task)
    {
        if (CDM_MUTANT != 1 && r->depth == r->maximum)
            return CMX_FULL;
        ++r->depth;
        return CMX_OK;
    }
    int result = cmx_mutex_lock(r->kernel, &r->mutex, task, d);
    if (result == CMX_OK)
        r->depth = 1;
    return result;
}

int cdm_recursive_unlock(struct cd_recursive *r, unsigned task)
{
    if (!r || task >= CMX_TASKS)
        return CMX_INVALID;
    if (r->mutex.owner != (int)task)
        return CMX_NOT_OWNER;
    if (r->depth > 1 && CDM_MUTANT != 2)
    {
        --r->depth;
        return CMX_OK;
    }
    int result = cmx_mutex_unlock(r->kernel, &r->mutex, task);
    if (result == CMX_OK)
        r->depth = r->mutex.owner < 0 ? 0 : 1;
    return result;
}

int cdm_recursive_close(struct cd_recursive *r)
{
    return r ? cmx_mutex_close(r->kernel, &r->mutex) : CMX_INVALID;
}

int cdm_ordered_lock(cmx_kernel *k, cmx_mutex *const *locks,
                     const unsigned *ranks, unsigned count, unsigned target,
                     unsigned task, cmx_deadline d)
{
    if (!live(k, task) || !locks || !ranks || !count || count > CMX_MUTEXES ||
        target >= count)
        return CMX_INVALID;
    /* The registry is a complete, immutable set for this ordering discipline. */
    if (count != k->mutex_count)
        return CMX_INVALID;
    for (unsigned i = 0; i < count; ++i)
    {
        if (!locks[i] || !registered(k, locks[i]))
            return CMX_INVALID;
        for (unsigned j = 0; j < i; ++j)
            if (locks[j] == locks[i] || ranks[j] == ranks[i])
                return CMX_INVALID;
    }
    for (unsigned i = 0; i < count; ++i)
        if (CDM_MUTANT != 3 && locks[i]->owner == (int)task &&
            ranks[target] <= ranks[i])
            return CMX_DEADLOCK;
    return cmx_mutex_lock(k, locks[target], task, d);
}

static uint32_t checksum(int32_t balance)
{
    return (uint32_t)balance ^ UINT32_C(0x9e3779b9);
}

void cdm_record_init(struct cd_record *r, int32_t balance)
{
    *r = (struct cd_record){balance, checksum(balance)};
}

int cdm_record_read(cmx_kernel *k, cmx_mutex *m, unsigned task,
                    const struct cd_record *r, struct cd_record *out)
{
    if (!k || !m || !r || !out || task >= CMX_TASKS)
        return CMX_INVALID;
    if (m->owner != (int)task)
        return CMX_NOT_OWNER;
    if (r->checksum != checksum(r->balance))
        return CMX_BROKEN;
    *out = *r;
    return CMX_OK;
}

int cdm_record_apply(cmx_kernel *k, cmx_mutex *m, unsigned task,
                     struct cd_record *r, int32_t delta,
                     cd_record_observer_fn observer, void *context)
{
    if (!k || !m || !r || task >= CMX_TASKS)
        return CMX_INVALID;
    if (m->owner != (int)task)
        return CMX_NOT_OWNER;
    if (r->checksum != checksum(r->balance))
        return CMX_BROKEN;
    int64_t next = (int64_t)r->balance + delta;
    if (next < INT32_MIN || next > INT32_MAX)
    {
        if (CDM_MUTANT == 5)
            r->checksum = 0;
        return CMX_FULL;
    }
    r->balance = (int32_t)next;
    if (CDM_MUTANT == 4)
    {
        (void)cmx_mutex_unlock(k, m, task);
        if (observer)
            observer(context);
    }
    r->checksum = checksum(r->balance);
    if (CDM_MUTANT != 4 && observer)
        observer(context);
    return CMX_OK;
}
