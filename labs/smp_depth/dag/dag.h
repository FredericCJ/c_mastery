#ifndef SMP_DAG_H
#define SMP_DAG_H
#include "locks.h"
#include <stddef.h>
enum {DG_OK,DG_INVALID,DG_CAPACITY,DG_CYCLE,DG_DUPLICATE,DG_RANGE,DG_CLOSED,DG_EMPTY,DG_STALE,DG_BUSY,DG_REQUESTED};
enum {DG_WAITING,DG_READY,DG_RUNNING,DG_DONE,DG_FAILED,DG_SKIPPED,DG_CANCELLED};
enum {DG_LIMIT=16,DG_EDGES=32};
typedef struct {unsigned from,to;} DgEdge;
typedef struct {unsigned count,edge_count;uint64_t costs[DG_LIMIT],payloads[DG_LIMIT];DgEdge edges[DG_EDGES];} DgSpec;
typedef struct {uint64_t payload,cost,input,result,span;uint16_t predecessors,successors;unsigned remaining,state,owner,cancel,error,ready_events;} DgNode;
typedef struct {uint64_t world,work,span;unsigned count,edges,sealed,terminal;SlLock lock;DgNode nodes[DG_LIMIT];} Dg;
typedef struct {uint64_t world,input,cost;unsigned node,worker;} DgClaim;
typedef struct {uint64_t world,work,span;unsigned count,edges,sealed,terminal,ready,running,states[DG_LIMIT],ready_events[DG_LIMIT];uint64_t results[DG_LIMIT];} DgSnapshot;
/* Stable caller-owned graph. All input/control/output extents are disjoint.
 * Initialization is exclusive and requires no surviving claim/caller from an
 * older world; the positive external world identity is never reused.
 * Initial arbitrary edges are copied/validated transactionally. Duplicate
 * edges and cycles are errors. Runtime append admits only a new node whose
 * unique predecessors already exist: no old-node or reverse-edge mutation.
 * Work/input arithmetic is checked; failure preserves admission outputs.
 * No callback executes under the lock. A claim is owned until finish. */
int dg_init(Dg *,uint64_t world,const DgSpec *);
int dg_append(Dg *,uint64_t payload,uint64_t cost,const unsigned *predecessors,unsigned count,unsigned *node);
int dg_seal(Dg *);
int dg_claim(Dg *,unsigned worker,DgClaim *);
/* outcome is DG_DONE or DG_FAILED; pending cancellation overrides either.
 * A successful task's result is supplied by its owner. The assessment uses
 * claim.input as the result, with cost controlling independent work only. */
int dg_finish(Dg *,DgClaim,unsigned outcome,uint64_t result);
int dg_cancel(Dg *,unsigned node);
int dg_snapshot(Dg *,DgSnapshot *);
int dg_drained(Dg *);
#endif
