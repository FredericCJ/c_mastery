#ifndef SMP_LOCKS_H
#define SMP_LOCKS_H
#include <stdint.h>
#include <stdatomic.h>
enum { SL_OK,SL_INVALID,SL_BUSY,SL_FULL,SL_PENDING,SL_ORDER };
enum { SL_TAS,SL_TICKET,SL_MCS };
typedef struct SlNode SlNode;
struct SlNode {_Atomic(SlNode *) next;atomic_uint waiting,active;};
typedef struct {unsigned kind,rank;atomic_uint word,next,serving,admitted;_Atomic(SlNode *) tail;} SlLock;
typedef struct {uint64_t exchanges,failed_exchanges,waiting_loads;} SlStats;
typedef struct {SlLock *lock;SlNode *node;unsigned ticket,phase;} SlGuard;
typedef struct SlIrqGuard SlIrqGuard;
struct SlIrqGuard {SlGuard guard;uint64_t saved;};
typedef struct {SlIrqGuard *stack[4];unsigned depth;} SlContext;
typedef struct {uint64_t (*save)(void *);void (*restore)(void *,uint64_t);void *context;} SlPort;
/* Initialize exclusively before sharing. All guards/contexts start zeroed.
 * A live guard is neither copied nor moved and is released only by its owner. */
int sl_init(SlLock *,unsigned kind,unsigned rank,unsigned start);
void sl_node_init(SlNode *);
int sl_ticket_reserve(SlLock *,SlGuard *);
int sl_ticket_ready(SlGuard *,SlStats *);
int sl_enter(SlLock *,SlNode *,SlGuard *,SlStats *);
int sl_leave(SlGuard *,SlStats *);
int sl_irq_enter(SlContext *,SlPort,SlLock *,SlNode *,SlIrqGuard *,SlStats *);
int sl_irq_leave(SlContext *,SlPort,SlIrqGuard *,SlStats *);
#endif
