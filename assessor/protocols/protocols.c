#include "protocols.h"

void pt_device_init(struct pt_device *d, struct pt_closed *out)
{
    d->generation = 1;
    d->state = PT_CLOSED;
    d->control = d->cleanup = 0;
    out->device = d;
    out->generation = 1;
}

int pt_configure(struct pt_closed *old, unsigned fail, struct pt_ready *out)
{
    if (!old || !out || !old->device || fail > 2)
    {
        return PT_INVALID;
    }
    struct pt_device *d = old->device;
    if (d->state != PT_CLOSED)
    {
        return PT_STATE;
    }
#ifndef TS_MUTATE_TYPESTATE2
    if (old->generation != d->generation)
    {
        return PT_STATE;
    }
#endif
    if (d->generation == UINT32_MAX)
    {
        return PT_RANGE;
    }
    d->generation++;
    old->device = 0;
    d->control = 1;
    if (fail == 1)
    {
        d->state = PT_RECOVERY_REQUIRED;
        return PT_RECOVERY;
    }
    d->control = 3;
    if (fail == 2)
    {
#ifdef TS_MUTATE_TYPESTATE
        d->state = PT_CLOSED;
#else
        d->state = PT_RECOVERY_REQUIRED;
#endif
        return PT_RECOVERY;
    }
    d->state = PT_READY;
    out->device = d;
    out->generation = d->generation;
    return PT_OK;
}

int pt_recover(struct pt_device *d, unsigned quiet, struct pt_closed *out)
{
    if (!d || !out || quiet > 1)
    {
        return PT_INVALID;
    }
    if (d->state != PT_RECOVERY_REQUIRED)
    {
        return PT_STATE;
    }
    if (!quiet)
    {
        return PT_PENDING;
    }
    if (d->generation == UINT32_MAX)
    {
        return PT_RANGE;
    }
    d->control = 0;
    d->cleanup++;
    d->state = PT_CLOSED;
    d->generation++;
    out->device = d;
    out->generation = d->generation;
    return PT_OK;
}

int pt_close(struct pt_ready *old, struct pt_closed *out)
{
    if (!old || !out || !old->device)
    {
        return PT_INVALID;
    }
    struct pt_device *d = old->device;
    if (d->state != PT_READY || old->generation != d->generation)
    {
        return PT_STATE;
    }
    if (d->generation == UINT32_MAX)
    {
        return PT_RANGE;
    }
    d->control = 0;
    d->cleanup++;
    d->state = PT_CLOSED;
    d->generation++;
    old->device = 0;
    out->device = d;
    out->generation = d->generation;
    return PT_OK;
}

int pt_builder_set(struct pt_builder *b, unsigned field, unsigned value)
{
    if (!b || field > 2 || b->mask > 7)
    {
        return PT_INVALID;
    }
    if (b->mask & (1u << field))
    {
        return PT_STATE;
    }
    if ((field == 0 && (value > 255 || value % 4)) ||
        (field == 1 && (!value || value > 64)) ||
        (field == 2 && (!value || value > 1000)))
    {
        return PT_RANGE;
    }
    if (field == 0)
    {
        b->address = value;
    }
    else if (field == 1)
    {
        b->length = value;
    }
    else
    {
        b->timeout = value;
    }
    b->mask |= 1u << field;
    return PT_OK;
}

int pt_builder_finish(const struct pt_builder *b, struct pt_config *out)
{
    if (!b || !out)
    {
        return PT_INVALID;
    }
#ifndef TS_MUTATE_BUILDER
    if (b->mask != 7)
    {
        return PT_STATE;
    }
#endif
#ifndef TS_MUTATE_BUILDER2
    if (b->address > 256 || b->length > 256 - b->address)
    {
        return PT_RANGE;
    }
#endif
    struct pt_config c = {b->address, b->length, b->timeout};
    *out = c;
    return PT_OK;
}

int pt_session_step(struct pt_session *s, unsigned sending, unsigned tag)
{
    if (!s || s->role > 1 || s->phase > 5 || s->remaining > 8 || sending > 1)
    {
        return PT_INVALID;
    }
    if (s->phase == 5)
    {
        return PT_STATE;
    }
    if ((s->phase == 2 || s->phase == 3) && !s->remaining)
    {
        return PT_INVALID;
    }
    static const unsigned tags[] = {PT_HELLO, PT_ACCEPT, PT_DATA, PT_ACK,
                                    PT_BYE};
    unsigned expected_send = (s->phase % 2 == 0) ^ (s->role == 1);
    (void)expected_send;
#ifndef TS_MUTATE_SESSION
    if (sending != expected_send || tag != tags[s->phase])
    {
        return PT_STATE;
    }
#else
    if (tag != tags[s->phase])
    {
        return PT_STATE;
    }
#endif
    if (s->phase == 1)
    {
        s->phase = s->remaining ? 2 : 4;
    }
    else if (s->phase == 3)
    {
#ifndef TS_MUTATE_SESSION2
        s->remaining--;
#endif
        s->phase = s->remaining ? 2 : 4;
    }
    else
    {
        s->phase++;
    }
    return PT_OK;
}

static unsigned parent(unsigned state)
{
    return state == PT_IDLE || state == PT_BUSY ? PT_ONLINE : PT_ROOT;
}

int pt_dispatch(struct pt_machine *m, unsigned event, unsigned char *trace,
                size_t capacity, size_t *used)
{
    if (!m || !used || (!trace && capacity) || m->active < PT_OFFLINE ||
        m->active > PT_FAULT || event > PT_PING)
    {
        return PT_INVALID;
    }
    if (m->dispatching)
    {
        return PT_STATE;
    }
    unsigned state = m->active, target = state;
    if (event == PT_PING)
    {
        if (m->pings == UINT32_MAX)
        {
            return PT_RANGE;
        }
        m->pings++;
        *used = 0;
        return PT_OK;
    }
    if (state == PT_OFFLINE && event == PT_CONNECT)
    {
        target = PT_IDLE;
    }
    else if ((state == PT_IDLE || state == PT_BUSY) && event == PT_START)
    {
        target = PT_BUSY;
    }
    else if (state == PT_BUSY && event == PT_FINISH)
    {
        target = PT_IDLE;
    }
    else if ((state == PT_IDLE || state == PT_BUSY || state == PT_ONLINE) &&
             event == PT_FAIL)
    {
        target = PT_FAULT;
    }
    else if (state == PT_FAULT && event == PT_RESET)
    {
        target = PT_OFFLINE;
    }
    else
    {
        return PT_STATE;
    }
    unsigned common = parent(state), target_path[3], depth = 0;
    unsigned char local[6];
    size_t count = 0;
    /* Declared tree depth<=2. Self-transition explicitly exits and enters leaf. */
    if (state != target)
    {
        common = state;
        for (;;)
        {
            unsigned t = target;
            while (t != common && t != PT_ROOT)
            {
                t = parent(t);
            }
            if (t == common)
            {
                break;
            }
            common = parent(common);
        }
    }
#ifndef TS_MUTATE_HSM2
    for (unsigned t = state; t != common; t = parent(t))
    {
        local[count++] = (unsigned char)(0x80u | t);
    }
#endif
    for (unsigned t = target; t != common; t = parent(t))
    {
        target_path[depth++] = t;
    }
#ifdef TS_MUTATE_HSM
    for (unsigned i = 0; i < depth; i++)
    {
        local[count++] = (unsigned char)target_path[i];
    }
#else
    while (depth)
    {
        local[count++] = (unsigned char)target_path[--depth];
    }
#endif
    if (count > capacity)
    {
        return PT_RANGE;
    }
    for (size_t i = 0; i < count; i++)
    {
        trace[i] = local[i];
    }
    m->active = target;
    *used = count;
    return PT_OK;
}

int pt_post(struct pt_active *a, unsigned event)
{
    if (!a || event > PT_PING || a->count > 4 || a->head > 3)
    {
        return PT_INVALID;
    }
#ifndef TS_MUTATE_ACTIVE
    if (a->count == 4)
    {
        return PT_RANGE;
    }
#endif
    a->queue[(a->head + a->count) % 4] = event;
    a->count++;
    return PT_OK;
}

int pt_active_step(struct pt_active *a, unsigned char *trace, size_t capacity,
                   size_t *used)
{
    if (!a || !used || a->count > 4 || a->head > 3)
    {
        return PT_INVALID;
    }
    if (!a->count)
    {
        return PT_PENDING;
    }
    int r = pt_dispatch(&a->machine, a->queue[a->head], trace, capacity, used);
#ifdef TS_MUTATE_ACTIVE2
    a->head = (a->head + 1) % 4;
    a->count--;
    return r;
#else
    if (r)
    {
        return r;
    }
    a->head = (a->head + 1) % 4;
    a->count--;
    return PT_OK;
#endif
}

int pt_thread_sum(struct pt_thread *p, unsigned available, uint32_t value,
                  uint32_t *out)
{
    if (!p || !out || available > 1 || p->count > 3)
    {
        return PT_INVALID;
    }
    if (p->pc == UINT32_MAX)
    {
        return PT_DONE;
    }
    if (p->pc > 1)
    {
        return PT_STATE;
    }
#ifdef TS_MUTATE_THREAD
    p->sum = 0;
#endif
    if (!available)
    {
        p->pc = 1;
        return PT_PENDING;
    }
#ifndef TS_MUTATE_THREAD2
    if (value > UINT32_MAX - p->sum)
    {
        return PT_RANGE;
    }
#endif
    p->sum += value;
    p->count++;
    p->pc = 1;
    if (p->count == 3)
    {
        *out = p->sum;
        p->pc = UINT32_MAX;
        return PT_DONE;
    }
    return PT_PENDING;
}

static int tree_valid(const struct pt_node *p, size_t n)
{
    unsigned parents[8] = {0};
    if (!n || n > 8)
    {
        return 0;
    }
    for (size_t i = 0; i < n; i++)
    {
        if (p[i].kind > PT_BT_SELECTOR)
        {
            return 0;
        }
        if (p[i].kind == PT_BT_LEAF)
        {
            if (p[i].count)
            {
                return 0;
            }
            continue;
        }
        if (!p[i].count || p[i].first <= i || p[i].first >= n ||
            p[i].count > n - p[i].first)
        {
            return 0;
        }
        for (unsigned j = 0; j < p[i].count; j++)
        {
            if (++parents[p[i].first + j] != 1)
            {
                return 0;
            }
        }
    }
    for (size_t i = 1; i < n; i++)
    {
        if (parents[i] != 1)
        {
            return 0;
        }
    }
    return 1;
}

static unsigned tick_node(struct pt_behavior *b, const struct pt_node *p,
                          unsigned node, pt_tick_fn tick, void *ctx)
{
    if (p[node].kind == PT_BT_LEAF)
    {
        unsigned r = tick(ctx, node);
        if (r == PT_BT_RUNNING)
        {
            b->active_mask |= 1u << node;
        }
        else
        {
            b->active_mask &= ~(1u << node);
        }
        return r;
    }
#ifdef TS_MUTATE_BEHAVIOR
    b->cursor[node] = 0;
#endif
    while (b->cursor[node] < p[node].count)
    {
        unsigned r =
            tick_node(b, p, p[node].first + b->cursor[node], tick, ctx);
        if (r > PT_BT_RUNNING)
        {
            return r;
        }
        if (r == PT_BT_RUNNING)
        {
            return r;
        }
        if ((p[node].kind == PT_BT_SEQUENCE && r == PT_BT_FAILURE) ||
            (p[node].kind == PT_BT_SELECTOR && r == PT_BT_SUCCESS))
        {
            b->cursor[node] = 0;
            return r;
        }
        b->cursor[node]++;
    }
    b->cursor[node] = 0;
    return p[node].kind == PT_BT_SEQUENCE ? PT_BT_SUCCESS : PT_BT_FAILURE;
}

int pt_behavior_tick(struct pt_behavior *b, const struct pt_node *p, size_t n,
                     pt_tick_fn tick, void *ctx, unsigned *out)
{
    if (!b || !p || !tick || !out || !tree_valid(p, n))
    {
        return PT_INVALID;
    }
    if (b->ticking)
    {
        return PT_STATE;
    }
    for (size_t i = 0; i < n; i++)
    {
        if (p[i].kind != PT_BT_LEAF && b->cursor[i] >= p[i].count)
        {
            return PT_INVALID;
        }
    }
    b->ticking = 1;
    unsigned r = tick_node(b, p, 0, tick, ctx);
    b->ticking = 0;
    if (r > PT_BT_RUNNING)
    {
        return PT_INVALID;
    }
    *out = r;
    return PT_OK;
}

int pt_behavior_cancel(struct pt_behavior *b, pt_abort_fn abort, void *ctx)
{
    if (!b || !abort)
    {
        return PT_INVALID;
    }
    if (b->ticking)
    {
        return PT_STATE;
    }
    b->ticking = 1;
    unsigned mask = b->active_mask;
    b->active_mask = 0;
    for (unsigned i = 0; i < 8; i++)
    {
        b->cursor[i] = 0;
#ifndef TS_MUTATE_BEHAVIOR2
        if (mask & (1u << i))
        {
            abort(ctx, i);
        }
#endif
    }
    (void)mask;
    (void)ctx;
    b->ticking = 0;
    return PT_OK;
}

int pt_driver_start(struct pt_driver *d, const unsigned char *p, size_t n)
{
    if (!d || !p || !n || n > 64 || !d->port.start_dma ||
        !d->port.stop_and_quiesce || !d->port.watchdog)
    {
        return PT_INVALID;
    }
    if (d->device.state != PT_READY || d->dma_owned)
    {
        return PT_STATE;
    }
    d->buffer = p;
    d->length = n;
    d->dma_owned = 1;
    if (d->port.start_dma(d->port.context, p, n))
    {
#ifdef TS_MUTATE_DRIVER
        d->buffer = 0;
        d->length = 0;
        d->dma_owned = 0;
#endif
        d->device.state = PT_RECOVERY_REQUIRED;
        return PT_RECOVERY;
    }
    return PT_OK;
}

int pt_driver_complete(struct pt_driver *d, unsigned error, unsigned quiet)
{
    if (!d || error > 1 || quiet > 1)
    {
        return PT_INVALID;
    }
    if (!d->dma_owned)
    {
        return PT_STATE;
    }
#ifndef TS_MUTATE_DRIVER2
    if (!quiet)
    {
        return PT_PENDING;
    }
#endif
    d->dma_owned = 0;
    d->buffer = 0;
    d->length = 0;
    if (error)
    {
        d->device.state = PT_RECOVERY_REQUIRED;
        return PT_RECOVERY;
    }
    if (d->device.state == PT_RECOVERY_REQUIRED)
    {
        return PT_RECOVERY;
    }
    d->port.watchdog(d->port.context);
    return PT_OK;
}

int pt_driver_recover(struct pt_driver *d)
{
    if (!d || !d->port.stop_and_quiesce)
    {
        return PT_INVALID;
    }
    if (d->device.state != PT_RECOVERY_REQUIRED)
    {
        return PT_STATE;
    }
    if (d->device.generation == UINT32_MAX)
    {
        return PT_RANGE;
    }
    if (d->port.stop_and_quiesce(d->port.context))
    {
        return PT_PENDING;
    }
    d->dma_owned = 0;
    d->buffer = 0;
    d->length = 0;
    struct pt_closed closed;
    return pt_recover(&d->device, 1, &closed);
}
