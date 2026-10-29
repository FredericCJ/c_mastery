#ifndef SMP_QUEUES_H
#define SMP_QUEUES_H
#include "locks.h"
enum { SQ_OK,SQ_INVALID,SQ_STALE,SQ_STATE,SQ_FULL,SQ_IDLE,SQ_DUPLICATE };
enum { SQ_EMPTY,SQ_READY,SQ_RUNNING,SQ_BLOCKED,SQ_EXITED,SQ_TRANSFER };
enum { SQ_CPUS=4,SQ_TASKS=16,SQ_CAPACITY=4,SQ_NONE=16 };
struct sq_handle {unsigned slot;uint64_t generation;};
struct sq_transfer {struct sq_handle task;unsigned actor,source,destination;uint64_t revision;};
struct sq_event {unsigned actor,slot,from,to,source,destination;uint64_t generation,revision;};
struct sq_task {atomic_uint state,home;uint64_t generation,revision;unsigned source,destination;};
struct sq_queue {struct sl_lock lock;unsigned slots[SQ_CAPACITY],count,reserved,current;};
struct sq_world {unsigned cpus,capacity,coarse;struct sl_lock global;struct sq_queue queue[SQ_CPUS];struct sq_task task[SQ_TASKS];};
/* Initialization creates an externally distinct world lifetime: ALL previous
 * handles/tickets/context pointers must be invalidated by the caller. It is
 * not a same-world reset. Slot creation/reuse preserves generation history.
 * Initialization and slot creation/reuse require whole-world quiescence.
 * Other operations admit one caller per actor CPU, with no queue operations
 * from interrupts. Stable handle/event storage must not alias the world.
 * Finish is called only after the executing context is saved and quiescent. */
int sq_init(struct sq_world *,unsigned cpus,unsigned capacity,unsigned coarse);
int sq_create(struct sq_world *,unsigned actor,unsigned home,unsigned slot,struct sq_handle *,struct sq_event *);
int sq_dispatch(struct sq_world *,unsigned cpu,struct sq_handle *,struct sq_event *,struct sl_stats *);
int sq_finish(struct sq_world *,unsigned cpu,struct sq_handle,unsigned next,struct sq_event *,struct sl_stats *);
int sq_wake(struct sq_world *,unsigned actor,unsigned home,struct sq_handle,struct sq_event *,struct sl_stats *);
int sq_transfer_begin(struct sq_world *,unsigned actor,unsigned source,unsigned destination,struct sq_handle,struct sq_transfer *,struct sq_event *,struct sl_stats *);
int sq_transfer_end(struct sq_world *,unsigned actor,struct sq_transfer,int commit,struct sq_event *,struct sl_stats *);
/* Diagnostic whole-world-quiescent snapshot invariant, not a concurrent scan. */
int sq_invariant(const struct sq_world *);
#endif
