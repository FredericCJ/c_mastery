#ifndef SMP_QUEUES_H
#define SMP_QUEUES_H
#include "locks.h"
enum { SQ_OK,SQ_INVALID,SQ_STALE,SQ_STATE,SQ_FULL,SQ_IDLE,SQ_DUPLICATE };
enum { SQ_EMPTY,SQ_READY,SQ_RUNNING,SQ_BLOCKED,SQ_EXITED,SQ_TRANSFER };
enum { SQ_CPUS=4,SQ_TASKS=16,SQ_CAPACITY=4,SQ_NONE=16 };
typedef struct {unsigned slot;uint64_t generation;} SqHandle;
typedef struct {SqHandle task;unsigned actor,source,destination;uint64_t revision;} SqTransfer;
typedef struct {unsigned actor,slot,from,to,source,destination;uint64_t generation,revision;} SqEvent;
typedef struct {atomic_uint state,home;uint64_t generation,revision;unsigned source,destination;} SqTask;
typedef struct {SlLock lock;unsigned slots[SQ_CAPACITY],count,reserved,current;} SqQueue;
typedef struct {unsigned cpus,capacity,coarse;SlLock global;SqQueue queue[SQ_CPUS];SqTask task[SQ_TASKS];} SqWorld;
/* Initialization creates an externally distinct world lifetime: ALL previous
 * handles/tickets/context pointers must be invalidated by the caller. It is
 * not a same-world reset. Slot creation/reuse preserves generation history.
 * Initialization and slot creation/reuse require whole-world quiescence.
 * Other operations admit one caller per actor CPU, with no queue operations
 * from interrupts. Stable handle/event storage must not alias the world.
 * Finish is called only after the executing context is saved and quiescent. */
int sq_init(SqWorld *,unsigned cpus,unsigned capacity,unsigned coarse);
int sq_create(SqWorld *,unsigned actor,unsigned home,unsigned slot,SqHandle *,SqEvent *);
int sq_dispatch(SqWorld *,unsigned cpu,SqHandle *,SqEvent *,SlStats *);
int sq_finish(SqWorld *,unsigned cpu,SqHandle,unsigned next,SqEvent *,SlStats *);
int sq_wake(SqWorld *,unsigned actor,unsigned home,SqHandle,SqEvent *,SlStats *);
int sq_transfer_begin(SqWorld *,unsigned actor,unsigned source,unsigned destination,SqHandle,SqTransfer *,SqEvent *,SlStats *);
int sq_transfer_end(SqWorld *,unsigned actor,SqTransfer,int commit,SqEvent *,SlStats *);
/* Diagnostic whole-world-quiescent snapshot invariant, not a concurrent scan. */
int sq_invariant(const SqWorld *);
#endif
