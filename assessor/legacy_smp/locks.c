#include "locks.h"
#include <stddef.h>
#ifndef SL_MUTANT
#define SL_MUTANT 0
#endif
static void pause_local(void)
{
    atomic_signal_fence(memory_order_seq_cst);
}

int sl_init(struct sl_lock *lock, unsigned kind, unsigned rank, unsigned start)
{
    if (!lock || kind > SL_MCS || !rank || start > 15 ||
        (kind != SL_TICKET && start))
        return SL_INVALID;
    lock->kind = kind;
    lock->rank = rank;
    atomic_init(&lock->word, 0);
    atomic_init(&lock->next, start);
    atomic_init(&lock->serving, start);
    atomic_init(&lock->admitted, 0);
    atomic_init(&lock->tail, NULL);
    return SL_OK;
}

void sl_node_init(struct sl_node *node)
{
    atomic_init(&node->next, NULL);
    atomic_init(&node->waiting, 0);
    atomic_init(&node->active, 0);
}

int sl_ticket_reserve(struct sl_lock *lock, struct sl_guard *guard)
{
    if (!lock || lock->kind != SL_TICKET || !guard || guard->phase)
        return SL_INVALID;
    unsigned active = atomic_load_explicit(
        &lock->admitted,
        SL_MUTANT == 6 ? memory_order_relaxed : memory_order_seq_cst);
    do
    {
        if (active >= 15 && SL_MUTANT != 1)
            return SL_FULL;
    } while (!atomic_compare_exchange_strong_explicit(
        &lock->admitted, &active, active + 1,
        SL_MUTANT == 6 ? memory_order_relaxed : memory_order_seq_cst,
        SL_MUTANT == 6 ? memory_order_relaxed : memory_order_seq_cst));
    unsigned ticket =
        atomic_fetch_add_explicit(&lock->next, 1,
                                  SL_MUTANT == 6 ? memory_order_relaxed
                                                 : memory_order_seq_cst) &
        15;
    *guard = (struct sl_guard){lock, NULL, ticket, 1};
    return SL_OK;
}

int sl_ticket_ready(struct sl_guard *guard, struct sl_stats *stats)
{
    if (!guard || !guard->lock || guard->lock->kind != SL_TICKET ||
        guard->phase != 1)
        return SL_INVALID;
    if (stats)
        stats->waiting_loads++;
    if ((atomic_load_explicit(&guard->lock->serving,
                              SL_MUTANT == 6 ? memory_order_acquire
                                             : memory_order_seq_cst) &
         15) != guard->ticket)
        return SL_PENDING;
    guard->phase = 2;
    return SL_OK;
}

int sl_enter(struct sl_lock *lock, struct sl_node *node, struct sl_guard *guard,
             struct sl_stats *stats)
{
    if (!lock || !guard || guard->phase || lock->kind > SL_MCS)
        return SL_INVALID;
    if (lock->kind == SL_TICKET)
    {
        int result = sl_ticket_reserve(lock, guard);
        if (result)
            return result;
        if (stats)
            stats->exchanges++;
        while (sl_ticket_ready(guard, stats) == SL_PENDING)
            pause_local();
        return SL_OK;
    }
    if (lock->kind == SL_TAS)
    {
        for (;;)
        {
            if (stats)
                stats->exchanges++;
            if (!atomic_exchange_explicit(&lock->word, 1, memory_order_acquire))
                break;
            if (stats)
                stats->failed_exchanges++;
            pause_local();
        }
    }
    else
    {
        if (!node)
            return SL_INVALID;
        unsigned empty = 0;
        if (!atomic_compare_exchange_strong_explicit(&node->active, &empty, 1,
                                                     memory_order_acq_rel,
                                                     memory_order_relaxed) &&
            SL_MUTANT != 5)
            return SL_BUSY;
        atomic_store_explicit(&node->next, NULL, memory_order_relaxed);
        atomic_store_explicit(&node->waiting, 1, memory_order_relaxed);
        struct sl_node *previous =
            atomic_exchange_explicit(&lock->tail, node, memory_order_acq_rel);
        if (stats)
            stats->exchanges++;
        if (previous)
        {
            atomic_store_explicit(&previous->next, node, memory_order_release);
            do
            {
                if (stats)
                    stats->waiting_loads++;
                if (!atomic_load_explicit(&node->waiting, memory_order_acquire))
                    break;
                pause_local();
            } while (1);
        }
    }
    *guard = (struct sl_guard){lock, node, 0, 2};
    return SL_OK;
}

int sl_leave(struct sl_guard *guard, struct sl_stats *stats)
{
    if (!guard || !guard->lock || guard->phase != 2)
        return SL_INVALID;
    struct sl_lock *lock = guard->lock;
    if (lock->kind == SL_TAS)
        atomic_store_explicit(&lock->word, 0,
                              SL_MUTANT == 4 ? memory_order_relaxed
                                             : memory_order_release);
    else if (lock->kind == SL_TICKET)
    {
        atomic_store_explicit(&lock->serving, (guard->ticket + 1) & 15,
                              SL_MUTANT == 4   ? memory_order_relaxed
                              : SL_MUTANT == 6 ? memory_order_release
                                               : memory_order_seq_cst);
        atomic_fetch_sub_explicit(&lock->admitted, 1,
                                  SL_MUTANT == 6 || SL_MUTANT == 4
                                      ? memory_order_relaxed
                                      : memory_order_seq_cst);
    }
    else
    {
        struct sl_node *node = guard->node,
                       *next = atomic_load_explicit(&node->next,
                                                    memory_order_acquire);
        if (!next)
        {
            struct sl_node *expected = node;
            if (stats)
                stats->exchanges++;
            if (atomic_compare_exchange_strong_explicit(
                    &lock->tail, &expected, NULL,
                    SL_MUTANT == 4 ? memory_order_relaxed
                                   : memory_order_release,
                    memory_order_relaxed))
                goto retired;
            if (stats)
                stats->failed_exchanges++;
            do
            {
                if (stats)
                    stats->waiting_loads++;
                next = atomic_load_explicit(&node->next, memory_order_acquire);
                if (!next)
                    pause_local();
            } while (!next);
        }
        atomic_store_explicit(&next->waiting, 0,
                              SL_MUTANT == 4 ? memory_order_relaxed
                                             : memory_order_release);
retired:
        atomic_store_explicit(&node->active, 0, memory_order_release);
    }
    guard->phase = 0;
    return SL_OK;
}

int sl_irq_enter(struct sl_context *context, struct sl_port port,
                 struct sl_lock *lock, struct sl_node *node,
                 struct sl_irq_guard *guard, struct sl_stats *stats)
{
    if (!context || !port.save || !port.restore || !lock || !guard ||
        guard->guard.phase)
        return SL_INVALID;
    uint64_t saved = port.save(port.context);
    if (context->depth == 4 ||
        (SL_MUTANT != 2 && context->depth &&
         context->stack[context->depth - 1]->guard.lock->rank >= lock->rank))
    {
        port.restore(port.context, saved);
        return SL_ORDER;
    }
    int result = sl_enter(lock, node, &guard->guard, stats);
    if (result)
    {
        port.restore(port.context, saved);
        return result;
    }
    guard->saved = saved;
    context->stack[context->depth++] = guard;
    return SL_OK;
}

int sl_irq_leave(struct sl_context *context, struct sl_port port,
                 struct sl_irq_guard *guard, struct sl_stats *stats)
{
    if (!context || !port.save || !port.restore || !guard || !context->depth ||
        context->stack[context->depth - 1] != guard)
        return SL_ORDER;
    uint64_t saved = guard->saved;
    int result = sl_leave(&guard->guard, stats);
    if (result)
        return result;
    context->stack[--context->depth] = NULL;
    port.restore(port.context, SL_MUTANT == 3 ? 1 : saved);
    return SL_OK;
}
