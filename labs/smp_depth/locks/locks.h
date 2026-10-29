#ifndef SMP_LOCKS_H
#define SMP_LOCKS_H
#include <stdint.h>
#include <stdatomic.h>

enum
{
    SL_OK,
    SL_INVALID,
    SL_BUSY,
    SL_FULL,
    SL_PENDING,
    SL_ORDER
};

enum
{
    SL_TAS,
    SL_TICKET,
    SL_MCS
};
struct sl_node;

struct sl_node
{
    _Atomic(struct sl_node *) next;
    atomic_uint waiting, active;
};

struct sl_lock
{
    unsigned kind, rank;
    atomic_uint word, next, serving, admitted;
    _Atomic(struct sl_node *) tail;
};

struct sl_stats
{
    uint64_t exchanges, failed_exchanges, waiting_loads;
};

struct sl_guard
{
    struct sl_lock *lock;
    struct sl_node *node;
    unsigned ticket, phase;
};
struct sl_irq_guard;

struct sl_irq_guard
{
    struct sl_guard guard;
    uint64_t saved;
};

struct sl_context
{
    struct sl_irq_guard *stack[4];
    unsigned depth;
};

struct sl_port
{
    uint64_t (*save)(void *);
    void (*restore)(void *, uint64_t);
    void *context;
};

/* Initialize exclusively before sharing. All guards/contexts start zeroed.
 * A live guard is neither copied nor moved and is released only by its owner. */
int sl_init(struct sl_lock *, unsigned kind, unsigned rank, unsigned start);
void sl_node_init(struct sl_node *);
int sl_ticket_reserve(struct sl_lock *, struct sl_guard *);
int sl_ticket_ready(struct sl_guard *, struct sl_stats *);
int sl_enter(struct sl_lock *, struct sl_node *, struct sl_guard *,
             struct sl_stats *);
int sl_leave(struct sl_guard *, struct sl_stats *);
int sl_irq_enter(struct sl_context *, struct sl_port, struct sl_lock *,
                 struct sl_node *, struct sl_irq_guard *, struct sl_stats *);
int sl_irq_leave(struct sl_context *, struct sl_port, struct sl_irq_guard *,
                 struct sl_stats *);
#endif
