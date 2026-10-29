#ifndef SMP_POOL_H
#define SMP_POOL_H
#include "locks.h"
#include <stddef.h>
enum {SP_OK,SP_INVALID,SP_FULL,SP_CLOSED,SP_EMPTY,SP_STALE,SP_BUSY,SP_REQUESTED,SP_FAILED,SP_DEGRADED,SP_NO_WORKERS};
enum {SP_FREE,SP_QUEUED,SP_RUNNING,SP_COMPLETED,SP_CANCELLED};
enum {SP_MAX_WORKERS=4,SP_QUEUE_CAPACITY=3,SP_JOBS=16};
struct sp_id {unsigned slot;uint64_t world,generation;};
struct sp_claim {struct sp_id id;unsigned worker;uint64_t payload,cost;};
struct sp_job {uint64_t generation,payload,cost,result;unsigned state,owner,queue,cancel;};
struct sp_queue {unsigned entries[SP_QUEUE_CAPACITY],count;};
struct sp_pool {uint64_t identity,accepted,completed,cancelled;unsigned workers,deques,closed;struct sl_lock lock;struct sp_job jobs[SP_JOBS];struct sp_queue queues[SP_MAX_WORKERS];};
struct sp_snapshot {uint64_t accepted,completed,cancelled;unsigned queued,running,terminal,queue_size[4];};
/* Stable caller-owned pool, positive externally unique world identity, no raw
 * state mutation. Initialization/reuse requires no surviving call or handle.
 * Shared policy: one FIFO capacity3. Deque policy: capacity3 per worker,
 * owner pops newest, other workers steal oldest under this same bounded lock.
 * Storage never moves/resizes; no lock-free or general reclamation claim. */
int sp_init(struct sp_pool *,uint64_t world,unsigned workers,int deques);
int sp_submit(struct sp_pool *,unsigned home,uint64_t payload,uint64_t cost,struct sp_id *);
int sp_claim(struct sp_pool *,unsigned worker,struct sp_claim *);
int sp_complete(struct sp_pool *,struct sp_claim,uint64_t result);
int sp_cancel(struct sp_pool *,struct sp_id);
int sp_result(struct sp_pool *,struct sp_id,unsigned *state,uint64_t *result);
int sp_collect(struct sp_pool *,struct sp_id);
int sp_close(struct sp_pool *,int cancel_accepted);
int sp_snapshot(struct sp_pool *,struct sp_snapshot *);

struct sp_launch_port {
 void *(*allocate)(void *,unsigned index);
 /* Failed create starts no worker. Opaque handle output is valid only on0. */
 int (*create)(void *,unsigned index,void *stack,uintptr_t *handle);
 /* Successful join establishes complete termination; failure retains authority. */
 int (*join)(void *,uintptr_t handle);
 void (*release)(void *,unsigned index,void *stack);
 void *context;
};
struct sp_launch {struct sp_launch_port port;atomic_uint gate;unsigned count,allocated,started,phase;void *stacks[4];uintptr_t handles[4];};
/* Sole controller, stable port/context and launch object. Workers acquire-load
 * gate:0 wait,1 run,2 stop. They must stop cooperatively and retain all borrowed
 * state until successful join. Callbacks never reenter launch operations.
 * Zero workers is a valid empty launch, not permission to accept queued work.
 * A degraded return retains every unjoined handle/stack for sp_shutdown retry. */
int sp_launch_init(struct sp_launch *,struct sp_launch_port);
int sp_launch(struct sp_launch *,unsigned workers);
int sp_shutdown(struct sp_launch *);
#endif
