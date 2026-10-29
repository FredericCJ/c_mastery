#include "once.h"
#ifndef CDO_MUTANT
#define CDO_MUTANT 0
#endif
static int valid(struct cdo_handle h)
{
    struct cd_once *o = h.owner;
    return o && o->live && h.task < 3 && h.epoch == o->epoch &&
           (o->registered & (1u << h.task)) &&
           o->slot[h.task].task_generation ==
               o->kernel->task[h.task].generation &&
           o->kernel->task[h.task].state != CMX_UNUSED &&
           o->kernel->task[h.task].state != CMX_EXITED;
}

static int owner(struct cdo_handle h)
{
    return valid(h) && h.owner->slot[h.task].phase == CDO_OWNER &&
           h.owner->wait.owner == (int)h.task;
}

static void cleanup(struct cd_once *o)
{
    while (o->resources)
    {
        unsigned i = --o->resources;
        struct cdo_cell *cell = o->resource[i];
        o->resource[i] = 0;
        if (CDO_MUTANT != 4 || i != 0)
            o->drop(o->allocator_context, cell);
    }
    o->table = (struct cdo_table){0};
    o->stage = 0;
}

static void settle(struct cd_once *o)
{
    for (unsigned i = 0; i < 3; i++)
        if (o->slot[i].phase == CDO_PENDING &&
            o->kernel->task[i].state != CMX_WAITING)
        {
            int result = o->kernel->task[i].result;
            if (result == CMX_RETRY)
                result = atomic_load_explicit(&o->publication.state,
                                              memory_order_acquire) == 5
                             ? CDO_FAILED
                             : CDO_AGAIN;
            o->slot[i].phase = CDO_DONE;
            o->slot[i].result = result;
        }
}

static int drained(struct cd_once *o)
{
    settle(o);
    for (unsigned i = 0; i < 3; i++)
        if (o->slot[i].phase != CDO_IDLE)
            return 0;
    return 1;
}

int cdo_init(struct cd_once *o, cmx_kernel *k, cdo_allocate_fn allocate,
             cdo_drop_fn drop, void *ctx)
{
    if (!o || !k || !allocate || !drop)
        return CMX_INVALID;
    /* Fresh/quiescent storage; no copying of an initialized atomic object. */
    o->kernel = k;
    o->table = (struct cdo_table){0};
    for (unsigned i = 0; i < 3; i++)
    {
        o->slot[i] = (struct cdo_slot){0};
        o->resource[i] = 0;
    }
    o->allocate = allocate;
    o->drop = drop;
    o->allocator_context = ctx;
    o->epoch = 1;
    o->attempts = 0;
    o->registered = o->stage = o->resources = o->resetting = o->closing = 0;
    o->live = 1;
    cmx_once_init(&o->wait);
    mb_once_init(&o->publication, false);
    return CMX_OK;
}

int cdo_register(struct cd_once *o, unsigned t, struct cdo_handle *out)
{
    if (!o || !o->live || !out || t >= 3 || o->resetting || o->closing ||
        (o->registered & (1u << t)) ||
        (o->kernel->task[t].state != CMX_READY &&
         o->kernel->task[t].state != CMX_RUNNING))
        return CMX_INVALID;
    o->registered |= 1u << t;
    o->slot[t].task_generation = o->kernel->task[t].generation;
    *out = (struct cdo_handle){o, t, o->epoch};
    return CMX_OK;
}

int cdo_enter(struct cdo_handle h, cmx_deadline d, struct cdo_ticket *out)
{
    if (!valid(h))
        return CDO_STALE;
    struct cd_once *o = h.owner;
    settle(o);
    struct cdo_slot *s = &o->slot[h.task];
    if (!out || o->resetting || o->closing)
        return CMX_INVALID;
    if (s->phase == CDO_OWNER)
        return CMX_DEADLOCK;
    if (s->phase != CDO_IDLE)
        return CMX_INVALID;
    if (o->kernel->task[h.task].state != CMX_READY &&
        o->kernel->task[h.task].state != CMX_RUNNING)
        return CMX_INVALID;
    if (s->serial == UINT32_MAX)
        return CDO_RETIRED;
    if (atomic_load_explicit(&o->publication.state, memory_order_acquire) ==
            0 &&
        o->attempts == UINT32_MAX)
        return CDO_RETIRED;
    unsigned values[2];
    int publication = mb_once_enter(&o->publication, h.task, values);
    int result;
    if (publication == MB_INIT)
    {
        result = cmx_once_enter(o->kernel, &o->wait, h.task, d);
        if (result != CMX_RETRY)
            return CMX_INVALID; /* Contract violation: producer states diverged. */
        o->attempts++;
        s->phase = CDO_OWNER;
        s->result = CDO_INIT;
        result = CDO_INIT;
    }
    else if (publication == MB_BUSY)
    {
        result = cmx_once_enter(o->kernel, &o->wait, h.task, d);
        if (result != CMX_BLOCKED)
            return result;
        s->phase = CDO_PENDING;
        s->result = result;
    }
    else if (publication == MB_OK)
    {
        s->phase = CDO_DONE;
        s->result = CMX_OK;
        result = CMX_OK;
    }
    else if (publication == MB_FAILED)
        return CDO_FAILED;
    else if (publication == MB_RECURSIVE)
        return CMX_DEADLOCK;
    else
        return publication == MB_RETRY ? CDO_AGAIN : CMX_INVALID;
    s->epoch = o->epoch;
    s->serial++;
    *out = (struct cdo_ticket){h, o->epoch, s->serial};
    return result;
}

int cdo_allocate(struct cdo_handle h)
{
    if (!owner(h))
        return CDO_STALE;
    struct cd_once *o = h.owner;
    if (o->resources || o->stage)
        return CMX_INVALID;
    for (unsigned i = 0; i < 3; i++)
    {
        struct cdo_cell *p = o->allocate(o->allocator_context);
        if (!p)
        {
            (void)cdo_finish(h, 0);
            return CDO_FAILED;
        }
        p->value = 0;
        o->resource[i] = p;
        o->resources++;
    }
    return CMX_OK;
}

int cdo_write(struct cdo_handle h, unsigned index, uint32_t value)
{
    if (!owner(h))
        return CDO_STALE;
    struct cd_once *o = h.owner;
    if (o->resources != 3 || index >= 8 || o->stage != index)
        return CMX_INVALID;
    o->table.entries[index] = value;
    o->resource[index % 3]->value += value;
    o->stage++;
    if (CDO_MUTANT == 1 && index == 0)
        (void)mb_once_finish(&o->publication, h.task, true, 0, 0);
    return CMX_OK;
}

int cdo_seal(struct cdo_handle h)
{
    if (!owner(h))
        return CDO_STALE;
    struct cd_once *o = h.owner;
    if (o->resources != 3 || (o->stage != 8 && o->stage != 9))
        return CMX_INVALID;
    if (o->stage == 8)
        o->table.count = 8;
    else
        o->table.checksum = o->resource[0]->value + o->resource[1]->value +
                            o->resource[2]->value;
    o->stage++;
    return CMX_OK;
}

int cdo_finish(struct cdo_handle h, unsigned success)
{
    if (!owner(h))
        return CDO_STALE;
    struct cd_once *o = h.owner;
    if (success > 1 || (success && o->stage != 10))
        return CMX_INVALID;
    if (!success)
        cleanup(o);
    int r = mb_once_finish(&o->publication, h.task, success != 0,
                           o->table.checksum, o->table.count);
    if (r != MB_OK)
        return CMX_INVALID;
    if (CDO_MUTANT == 2 && !success)
        mb_once_init(&o->publication, false);
    if (CDO_MUTANT != 3)
    {
        r = cmx_once_finish(o->kernel, &o->wait, h.task, success);
        if (r != CMX_OK)
            return CMX_INVALID;
    }
    o->slot[h.task].phase = CDO_DONE;
    o->slot[h.task].result = success ? CMX_OK : CDO_FAILED;
    settle(o);
    return CMX_OK;
}

int cdo_read(struct cdo_handle h, struct cdo_table *out)
{
    if (!valid(h))
        return CDO_STALE;
    if (!out)
        return CMX_INVALID;
    struct cd_once *o = h.owner;
    if (o->closing || o->resetting)
        return CMX_CLOSED;
    unsigned state =
        atomic_load_explicit(&o->publication.state, memory_order_acquire);
    if (state == 5)
        return CDO_FAILED;
    if (state != 4)
        return CMX_BLOCKED;
    *out = o->table;
    return CMX_OK;
}

int cdo_result(struct cdo_ticket t, struct cdo_table *out)
{
    if (!valid(t.handle) || t.epoch != t.handle.owner->epoch)
        return CDO_STALE;
    struct cd_once *o = t.handle.owner;
    settle(o);
    struct cdo_slot *s = &o->slot[t.handle.task];
    if (!out)
        return CMX_INVALID;
    if (s->phase == CDO_IDLE || s->phase == CDO_OWNER || s->epoch != t.epoch ||
        s->serial != t.serial)
        return CDO_STALE;
    if (s->phase == CDO_PENDING)
        return CMX_BLOCKED;
    int r = s->result;
    if (r == CMX_OK)
    {
        r = cdo_read(t.handle, out);
        if (r != CMX_OK)
            return r;
    }
    s->phase = CDO_IDLE;
    return r;
}

int cdo_cancel(struct cdo_ticket t)
{
    if (!valid(t.handle) || t.epoch != t.handle.owner->epoch)
        return CDO_STALE;
    struct cd_once *o = t.handle.owner;
    settle(o);
    struct cdo_slot *s = &o->slot[t.handle.task];
    if (s->phase == CDO_IDLE || s->epoch != t.epoch || s->serial != t.serial)
        return CDO_STALE;
    if (s->phase == CDO_OWNER)
        return cdo_finish(t.handle, 0);
    if (s->phase == CDO_PENDING)
    {
        int r = cmx_cancel(o->kernel, t.handle.task);
        if (r != CMX_OK)
            return r;
        settle(o);
    }
    return s->result;
}

int cdo_prepare_reset(struct cd_once *o)
{
    if (!o || !o->live || o->closing || o->resetting)
        return CMX_INVALID;
    if (atomic_load_explicit(&o->publication.state, memory_order_acquire) != 5)
        return CMX_INVALID;
    if (!drained(o) || o->resources)
        return CMX_BLOCKED;
    o->resetting = 1;
    return CMX_OK;
}

int cdo_release(struct cdo_handle *h)
{
    if (!h || !valid(*h))
        return CDO_STALE;
    struct cd_once *o = h->owner;
    if ((!o->resetting && !o->closing) || o->slot[h->task].phase != CDO_IDLE)
        return CMX_BLOCKED;
    o->registered &= ~(1u << h->task);
    *h = (struct cdo_handle){0};
    return CMX_OK;
}

int cdo_reset(struct cd_once *o)
{
    if (!o || !o->live || !o->resetting || o->closing)
        return CMX_INVALID;
    if ((CDO_MUTANT != 5 && o->registered) || !drained(o) || o->resources)
        return CMX_BLOCKED;
    if (o->epoch == UINT32_MAX || o->attempts == UINT32_MAX)
        return CDO_RETIRED;
    o->epoch++;
    o->resetting = 0;
    for (unsigned i = 0; i < 3; i++)
        o->slot[i].serial = 0;
    cmx_once_init(&o->wait);
    mb_once_init(&o->publication, false);
    return CMX_OK;
}

int cdo_close(struct cd_once *o)
{
    if (!o || !o->live)
        return CMX_INVALID;
    if (!drained(o))
        return CMX_BLOCKED;
    o->closing = 1;
    o->resetting = 0;
    return CMX_OK;
}

int cdo_destroy(struct cd_once *o)
{
    if (!o || !o->live)
        return CMX_INVALID;
    if (!o->closing || o->registered || !drained(o))
        return CMX_BLOCKED;
    cleanup(o);
    o->live = 0;
    return CMX_OK;
}
