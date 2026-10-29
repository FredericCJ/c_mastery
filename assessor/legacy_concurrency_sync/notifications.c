#include "notifications.h"
#ifndef CDN_MUTANT
#define CDN_MUTANT 0
#endif
static int valid(struct cdn_handle h)
{
    struct cd_notify *n = h.owner;
    return n && n->kernel && n->attached && n->task < CMX_TASKS &&
           (CDN_MUTANT == 3 ||
            (h.generation == n->generation &&
             n->kernel->task[n->task].generation == n->generation)) &&
           n->kernel->task[n->task].state != CMX_UNUSED &&
           n->kernel->task[n->task].state != CMX_EXITED;
}

static void settle(struct cd_notify *n)
{
    cmx_task *t = &n->kernel->task[n->task];
    if (n->phase == CDN_PENDING && t->state != CMX_WAITING)
    {
        n->phase = CDN_DONE;
        n->result = t->result;
        if (t->result == CMX_OK)
        {
            n->claimed = 1;
        }
    }
}

void cdn_init(struct cd_notify *n, cmx_kernel *k)
{
    *n = (struct cd_notify){0};
    n->kernel = k;
}

int cdn_attach(struct cd_notify *n, unsigned task, unsigned maximum,
               struct cdn_handle *out)
{
    if (!n || !n->kernel || !out || !maximum || n->attached ||
        task >= CMX_TASKS)
    {
        return CMX_INVALID;
    }
    cmx_task *t = &n->kernel->task[task];
    if ((t->state != CMX_READY && t->state != CMX_RUNNING) ||
        t->notifications ||
        (n->seen && (task != n->task || t->generation == n->generation)))
    {
        return CMX_INVALID;
    }
    cmx_kernel *k = n->kernel;
    *n = (struct cd_notify){0};
    n->kernel = k;
    n->task = task;
    n->generation = t->generation;
    n->maximum = maximum;
    n->attached = n->seen = 1;
    *out = (struct cdn_handle){n, n->generation};
    return CMX_OK;
}

int cdn_post(struct cdn_handle h, uint32_t count, struct cdn_post *out)
{
    if (!valid(h))
    {
        return CDN_STALE;
    }
    if (!count || !out)
    {
        return CMX_INVALID;
    }
    struct cd_notify *n = h.owner;
    cmx_task *t = &n->kernel->task[n->task];
    settle(n);
    if (n->closed)
    {
        return CMX_CLOSED;
    }
    if (n->fault_pending)
    {
        return CDN_FAULT_PENDING;
    }
    if (count > UINT64_MAX - n->accepted || count > UINT64_MAX - n->rejected ||
        n->fault_sequence == UINT64_MAX)
    {
        return CDN_RETIRED;
    }
    if (count > n->maximum - t->notifications)
    {
        if (CDN_MUTANT == 1)
        {
            count = n->maximum - t->notifications;
            if (!count)
            {
                return CMX_OK;
            }
        }
        else
        {
            n->rejected += count;
            n->fault = (struct cdn_fault){++n->fault_sequence, count, CMX_FULL};
            n->fault_pending = CDN_MUTANT != 6;
            return CMX_FULL;
        }
    }
    unsigned woke = n->phase == CDN_PENDING && t->state == CMX_WAITING &&
                    t->wait == CMX_W_NOTIFY;
    int result = cmx_notify(n->kernel, n->task, count);
    if (result != CMX_OK)
    {
        return result;
    }
    n->accepted += count;
    if (CDN_MUTANT == 2 && woke)
    {
        t->notifications = 0;
    }
    if (CDN_MUTANT == 4 && t->notifications > 1)
    {
        t->notifications = 1;
    }
    settle(n);
    *out = (struct cdn_post){woke, woke, 1};
    return CMX_OK;
}

int cdn_wait(struct cdn_handle h, uint32_t up_to, cmx_deadline deadline)
{
    if (!valid(h))
    {
        return CDN_STALE;
    }
    struct cd_notify *n = h.owner;
    cmx_task *t = &n->kernel->task[n->task];
    if (!up_to || n->phase != CDN_IDLE ||
        (t->state != CMX_READY && t->state != CMX_RUNNING))
    {
        return CMX_INVALID;
    }
    if (n->closed)
    {
        return CMX_CLOSED;
    }
    int result = cmx_notify_take(n->kernel, n->task, deadline);
    n->phase = result == CMX_BLOCKED ? CDN_PENDING : CDN_DONE;
    n->requested = up_to;
    n->claimed = result == CMX_OK;
    n->result = result;
    return result;
}

int cdn_result(struct cdn_handle h, uint32_t *count)
{
    if (!valid(h))
    {
        return CDN_STALE;
    }
    struct cd_notify *n = h.owner;
    if (!count || n->phase == CDN_IDLE)
    {
        return CMX_INVALID;
    }
    settle(n);
    if (n->phase == CDN_PENDING)
    {
        return CMX_BLOCKED;
    }
    int result = n->result;
    if (result == CMX_OK)
    {
        cmx_task *t = &n->kernel->task[n->task];
        uint32_t extra = n->requested - n->claimed;
        if (extra > t->notifications)
        {
            extra = t->notifications;
        }
        if (CDN_MUTANT == 5)
        {
            extra = t->notifications;
        }
        t->notifications -= extra;
        n->claimed += extra;
        n->consumed += n->claimed;
        *count = n->claimed;
    }
    n->claimed = 0;
    n->phase = CDN_IDLE;
    return result;
}

int cdn_cancel(struct cdn_handle h)
{
    if (!valid(h))
    {
        return CDN_STALE;
    }
    struct cd_notify *n = h.owner;
    settle(n);
    if (n->phase == CDN_IDLE)
    {
        return CMX_INVALID;
    }
    if (n->phase == CDN_PENDING)
    {
        int result = cmx_cancel(n->kernel, n->task);
        if (result != CMX_OK)
        {
            return result;
        }
        settle(n);
    }
    return n->result;
}

int cdn_close(struct cdn_handle h)
{
    if (!valid(h))
    {
        return CDN_STALE;
    }
    struct cd_notify *n = h.owner;
    settle(n);
    if (n->phase == CDN_PENDING)
    {
        (void)cmx_cancel(n->kernel, n->task);
        settle(n);
    }
    n->cancelled += n->kernel->task[n->task].notifications;
    n->kernel->task[n->task].notifications = 0;
    n->closed = 1;
    return CMX_OK;
}

int cdn_detach(struct cdn_handle h)
{
    if (!valid(h))
    {
        return CDN_STALE;
    }
    struct cd_notify *n = h.owner;
    if (!n->closed || n->phase != CDN_IDLE || n->fault_pending)
    {
        return CMX_BLOCKED;
    }
    n->attached = 0;
    return CMX_OK;
}

int cdn_fault(struct cdn_handle h, struct cdn_fault *out)
{
    if (!valid(h))
    {
        return CDN_STALE;
    }
    if (!out)
    {
        return CMX_INVALID;
    }
    if (!h.owner->fault_pending)
    {
        return CMX_BLOCKED;
    }
    *out = h.owner->fault;
    return CMX_OK;
}

int cdn_ack_fault(struct cdn_handle h, uint64_t sequence)
{
    if (!valid(h))
    {
        return CDN_STALE;
    }
    if (!h.owner->fault_pending || h.owner->fault.sequence != sequence)
    {
        return CDN_STALE;
    }
    h.owner->fault_pending = 0;
    return CMX_OK;
}

int cdn_invariant(const struct cd_notify *n)
{
    if (!n || !n->kernel || n->task >= CMX_TASKS)
    {
        return 0;
    }
    uint64_t remaining = n->kernel->task[n->task].notifications;
    if (remaining > n->maximum || n->claimed > 1 || n->consumed > n->accepted ||
        n->cancelled > n->accepted - n->consumed)
    {
        return 0;
    }
    return n->accepted - n->consumed - n->cancelled == remaining + n->claimed;
}
