#ifndef CMASTERY_ADP_H
#define CMASTERY_ADP_H
#include "adt.h"
#include <pthread.h>
typedef enum { ADP_OK,ADP_INVALID,ADP_FULL,ADP_STALE,ADP_CLOSED,ADP_NOMEM,ADP_STATE,ADP_SYNC } AdpStatus;
typedef struct {uint64_t value;} AdpId;
typedef struct {uint64_t next;unsigned exhausted;} AdpIds;
typedef struct {AdpId pool;unsigned index;uint64_t generation;} AdpHandle;
typedef void *(*AdpAllocate)(void *,size_t);
typedef void (*AdpDeallocate)(void *,void *);
typedef struct {AdpAllocate allocate;AdpDeallocate deallocate;void *context;} AdpOps;
typedef enum {ADP_FREE,ADP_LIVE,ADP_RETIRED} AdpState;
typedef struct {uint64_t generation;AdpState state;uint32_t value;} AdpSlot;
struct AdpBlock;
typedef struct {pthread_mutex_t mutex;AdpId identity;struct AdpBlock *block;AdpOps ops;} AdpPool;
typedef struct {AdpId identity;AdpSlot slots[4];} AdpSnapshot;
/* ID source externally serialized; never reuse an issued identity while any
   handle can survive, including process restart. No implicit durable issuer. */
AdpStatus adp_ids_init(AdpIds *,uint64_t first);
AdpStatus adp_issue(AdpIds *,AdpId *);
/* Init/move destination: unowned storage. Initial generation is positive;
   ordinary deployments use1; changed-generation fixtures may use another. */
AdpStatus adp_init(AdpPool *,AdpId,AdpOps,uint64_t generation);
AdpStatus adp_acquire(AdpPool *,uint32_t,AdpHandle *);
AdpStatus adp_get(AdpPool *,AdpHandle,uint32_t *);
AdpStatus adp_release(AdpPool *,AdpHandle);
AdpStatus adp_snapshot(AdpPool *,AdpSnapshot *);
/* Payload relocation is serialized with readers; no raw slot pointer escapes. */
AdpStatus adp_compact(AdpPool *);
/* Shell relocation requires quiescence of all source users/waiters; initializes
   destination mutex afresh, transfers block ownership, leaves source closed. */
AdpStatus adp_move(AdpPool *destination,AdpPool *source);
/* Close admits concurrent operations; destroy requires closed + all callers
   joined/quiescent. A closed pool retains a live mutex until destroy. */
AdpStatus adp_close(AdpPool *);
AdpStatus adp_destroy(AdpPool *);
#endif
