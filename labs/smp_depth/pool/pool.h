#ifndef SMP_POOL_H
#define SMP_POOL_H
#include "locks.h"
#include <stddef.h>
enum {SP_OK,SP_INVALID,SP_FULL,SP_CLOSED,SP_EMPTY,SP_STALE,SP_BUSY,SP_REQUESTED,SP_FAILED,SP_DEGRADED,SP_NO_WORKERS};
enum {SP_FREE,SP_QUEUED,SP_RUNNING,SP_COMPLETED,SP_CANCELLED};
enum {SP_MAX_WORKERS=4,SP_QUEUE_CAPACITY=3,SP_JOBS=16};
typedef struct {unsigned slot;uint64_t world,generation;} SpId;
typedef struct {SpId id;unsigned worker;uint64_t payload,cost;} SpClaim;
typedef struct {uint64_t generation,payload,cost,result;unsigned state,owner,queue,cancel;} SpJob;
typedef struct {unsigned entries[SP_QUEUE_CAPACITY],count;} SpQueue;
typedef struct {uint64_t identity,accepted,completed,cancelled;unsigned workers,deques,closed;SlLock lock;SpJob jobs[SP_JOBS];SpQueue queues[SP_MAX_WORKERS];} SpPool;
typedef struct {uint64_t accepted,completed,cancelled;unsigned queued,running,terminal,queue_size[4];} SpSnapshot;
/* Stable caller-owned pool, positive externally unique world identity, no raw
 * state mutation. Initialization/reuse requires no surviving call or handle.
 * Shared policy: one FIFO capacity3. Deque policy: capacity3 per worker,
 * owner pops newest, other workers steal oldest under this same bounded lock.
 * Storage never moves/resizes; no lock-free or general reclamation claim. */
int sp_init(SpPool *,uint64_t world,unsigned workers,int deques);
int sp_submit(SpPool *,unsigned home,uint64_t payload,uint64_t cost,SpId *);
int sp_claim(SpPool *,unsigned worker,SpClaim *);
int sp_complete(SpPool *,SpClaim,uint64_t result);
int sp_cancel(SpPool *,SpId);
int sp_result(SpPool *,SpId,unsigned *state,uint64_t *result);
int sp_collect(SpPool *,SpId);
int sp_close(SpPool *,int cancel_accepted);
int sp_snapshot(SpPool *,SpSnapshot *);

typedef struct {
 void *(*allocate)(void *,unsigned index);
 /* Failed create starts no worker. Opaque handle output is valid only on0. */
 int (*create)(void *,unsigned index,void *stack,uintptr_t *handle);
 /* Successful join establishes complete termination; failure retains authority. */
 int (*join)(void *,uintptr_t handle);
 void (*release)(void *,unsigned index,void *stack);
 void *context;
} SpLaunchPort;
typedef struct {SpLaunchPort port;atomic_uint gate;unsigned count,allocated,started,phase;void *stacks[4];uintptr_t handles[4];} SpLaunch;
/* Sole controller, stable port/context and launch object. Workers acquire-load
 * gate:0 wait,1 run,2 stop. They must stop cooperatively and retain all borrowed
 * state until successful join. Callbacks never reenter launch operations.
 * Zero workers is a valid empty launch, not permission to accept queued work.
 * A degraded return retains every unjoined handle/stack for sp_shutdown retry. */
int sp_launch_init(SpLaunch *,SpLaunchPort);
int sp_launch(SpLaunch *,unsigned workers);
int sp_shutdown(SpLaunch *);
#endif
