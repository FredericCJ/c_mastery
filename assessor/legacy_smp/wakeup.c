#include "wakeup.h"
#include <stddef.h>
#ifndef SW_MUTANT
#define SW_MUTANT 0
#endif
static int sw_valid(const struct sw_world *w, unsigned a, unsigned b)
{
    return w && w->queues.cpus && w->queues.cpus <= SQ_CPUS &&
           a < w->queues.cpus && b < w->queues.cpus;
}

static int sw_enter(struct sw_world *w, struct sl_guard *g, struct sl_stats *s)
{
    *g = (struct sl_guard){0};
    return sl_enter(&w->control, NULL, g, s) == SL_OK;
}

static void sw_publish(struct sw_world *w, unsigned c)
{
    atomic_store_explicit(&w->work[c], w->queues.queue[c].count,
                          memory_order_release);
}

static int sw_ring(struct sw_world *w, unsigned c, int remote)
{
    if (!w->queues.queue[c].count)
        return SQ_OK;
    unsigned old = w->bell[c];
    w->bell[c] = 1;
    if (!old && remote)
    {
        atomic_thread_fence(memory_order_seq_cst);
        if (!w->port.ring(w->port.context, c))
            return SW_SIGNAL;
    }
    return SQ_OK;
}

int sw_init(struct sw_world *w, unsigned n, unsigned capacity,
            struct sw_port port)
{
    if (!w || !port.ring || !port.ack)
        return SQ_INVALID;
    int r = sq_init(&w->queues, n, capacity, 0);
    if (r != SQ_OK)
        return r;
    (void)sl_init(&w->control, SL_TAS, 1, 0);
    w->port = port;
    for (unsigned c = 0; c < SQ_CPUS; c++)
    {
        w->bell[c] = 0;
        atomic_init(&w->work[c], 0);
    }
    return SQ_OK;
}

int sw_create(struct sw_world *w, unsigned actor, unsigned home, unsigned slot,
              struct sq_handle *id, struct sq_event *event)
{
    if (!sw_valid(w, actor, home))
        return SQ_INVALID;
    int r = sq_create(&w->queues, actor, home, slot, id, event);
    if (r == SQ_OK)
    {
        sw_publish(w, home);
        w->bell[home] = 1;
    }
    return r;
}

int sw_wake(struct sw_world *w, unsigned actor, unsigned home,
            struct sq_handle id, struct sq_event *event, struct sl_stats *s)
{
    if (!sw_valid(w, actor, home) || id.slot >= SQ_TASKS || !event)
        return SQ_INVALID;
    struct sl_guard guard;
    if (!sw_enter(w, &guard, s))
        return SQ_STATE;
    struct sq_task *t = &w->queues.task[id.slot];
    unsigned state = atomic_load_explicit(&t->state, memory_order_acquire);
    int r;
    if (!id.generation || t->generation != id.generation ||
        atomic_load_explicit(&t->home, memory_order_acquire) != home)
        r = SQ_STALE;
    else if (state == SQ_TRANSFER)
        r = SW_MOVING;
    else if (state == SQ_RUNNING)
        r = SQ_DUPLICATE;
    else
    {
        if (SW_MUTANT == 1 && state == SQ_BLOCKED)
            (void)w->port.ring(w->port.context, home);
        r = sq_wake(&w->queues, actor, home, id, event, s);
        if (r == SQ_OK)
        {
            sw_publish(w, home);
            if (SW_MUTANT == 1)
                w->bell[home] = 1;
            else
                r = sw_ring(w, home, 1);
        }
    }
    (void)sl_leave(&guard, s);
    return r;
}

int sw_dispatch(struct sw_world *w, unsigned cpu, struct sq_handle *id,
                struct sq_event *event, struct sl_stats *s)
{
    if (!sw_valid(w, cpu, cpu))
        return SQ_INVALID;
    struct sl_guard guard;
    if (!sw_enter(w, &guard, s))
        return SQ_STATE;
    int r = sq_dispatch(&w->queues, cpu, id, event, s);
    if (r == SQ_OK)
        sw_publish(w, cpu);
    (void)sl_leave(&guard, s);
    return r;
}

int sw_finish(struct sw_world *w, unsigned cpu, struct sq_handle id,
              unsigned next, struct sq_event *event, struct sl_stats *s)
{
    if (!sw_valid(w, cpu, cpu))
        return SQ_INVALID;
    struct sl_guard guard;
    if (!sw_enter(w, &guard, s))
        return SQ_STATE;
    int r = sq_finish(&w->queues, cpu, id, next, event, s);
    if (r == SQ_OK)
    {
        sw_publish(w, cpu);
        (void)sw_ring(w, cpu, 0);
    }
    (void)sl_leave(&guard, s);
    return r;
}

int sw_transfer_begin(struct sw_world *w, unsigned actor, unsigned src,
                      unsigned dst, struct sq_handle id,
                      struct sq_transfer *ticket, struct sq_event *event,
                      struct sl_stats *s)
{
    if (!sw_valid(w, actor, src) || dst >= w->queues.cpus)
        return SQ_INVALID;
    struct sl_guard guard;
    if (!sw_enter(w, &guard, s))
        return SQ_STATE;
    int r =
        sq_transfer_begin(&w->queues, actor, src, dst, id, ticket, event, s);
    if (r == SQ_OK)
    {
        sw_publish(w, src);
        sw_publish(w, dst);
    }
    (void)sl_leave(&guard, s);
    return r;
}

int sw_transfer_end(struct sw_world *w, unsigned actor,
                    struct sq_transfer ticket, int commit,
                    struct sq_event *event, struct sl_stats *s)
{
    if (!sw_valid(w, actor, ticket.source) ||
        ticket.destination >= w->queues.cpus)
        return SQ_INVALID;
    struct sl_guard guard;
    if (!sw_enter(w, &guard, s))
        return SQ_STATE;
    int r = sq_transfer_end(&w->queues, actor, ticket, commit, event, s);
    if (r == SQ_OK)
    {
        sw_publish(w, ticket.source);
        sw_publish(w, ticket.destination);
        r = sw_ring(w, commit ? ticket.destination : ticket.source, 1);
    }
    (void)sl_leave(&guard, s);
    return r;
}

int sw_retry(struct sw_world *w, unsigned actor, unsigned dst,
             struct sl_stats *s)
{
    if (!sw_valid(w, actor, dst))
        return SQ_INVALID;
    struct sl_guard guard;
    if (!sw_enter(w, &guard, s))
        return SQ_STATE;
    int r = SQ_IDLE;
    if (w->queues.queue[dst].count)
    {
        w->bell[dst] = 1;
        atomic_thread_fence(memory_order_seq_cst);
        r = w->port.ring(w->port.context, dst) ? SQ_OK : SW_SIGNAL;
    }
    (void)sl_leave(&guard, s);
    return r;
}

int sw_prepare_idle(struct sw_world *w, unsigned cpu, int masked,
                    struct sl_stats *s)
{
    if (!sw_valid(w, cpu, cpu) || masked != 1)
        return SQ_INVALID;
    struct sl_guard guard;
    if (!sw_enter(w, &guard, s))
        return SQ_STATE;
    /* Deassert the level, then check durable work while publication is excluded.
     * A producer after unlock reasserts; a pending fixed IPI survives IF=0. */
    w->port.ack(w->port.context, cpu);
    w->bell[cpu] = 0;
    int r;
    if (w->queues.queue[cpu].current != SQ_NONE)
        r = SW_BUSY;
    else if (SW_MUTANT != 2 && w->queues.queue[cpu].count)
    {
        r = sw_ring(w, cpu, 1);
        if (r == SQ_OK)
            r = SW_BUSY;
    }
    else
        r = SQ_IDLE;
    (void)sl_leave(&guard, s);
    return r;
}
