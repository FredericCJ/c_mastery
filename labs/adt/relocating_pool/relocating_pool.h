#ifndef CMASTERY_ADP_H
#define CMASTERY_ADP_H
#include "adt.h"
#include <pthread.h>
enum adp_status { ADP_OK,ADP_INVALID,ADP_FULL,ADP_STALE,ADP_CLOSED,ADP_NOMEM,ADP_STATE,ADP_SYNC };
struct adp_id {uint64_t value;};
struct adp_ids {uint64_t next;unsigned exhausted;};
struct adp_handle {struct adp_id pool;unsigned index;uint64_t generation;};
typedef void *(*adp_allocate_fn)(void *,size_t);
typedef void (*adp_deallocate_fn)(void *,void *);
struct adp_ops {adp_allocate_fn allocate;adp_deallocate_fn deallocate;void *context;};
enum adp_state {ADP_FREE,ADP_LIVE,ADP_RETIRED};
struct adp_slot {uint64_t generation;enum adp_state state;uint32_t value;};
struct adp_block;
struct adp_pool {pthread_mutex_t mutex;struct adp_id identity;struct adp_block *block;struct adp_ops ops;};
struct adp_snapshot {struct adp_id identity;struct adp_slot slots[4];};
/* ID source externally serialized; never reuse an issued identity while any
   handle can survive, including process restart. No implicit durable issuer. */
enum adp_status adp_ids_init(struct adp_ids *,uint64_t first);
enum adp_status adp_issue(struct adp_ids *,struct adp_id *);
/* Init/move destination: unowned storage. Initial generation is positive;
   ordinary deployments use1; changed-generation fixtures may use another. */
enum adp_status adp_init(struct adp_pool *,struct adp_id,struct adp_ops,uint64_t generation);
enum adp_status adp_acquire(struct adp_pool *,uint32_t,struct adp_handle *);
enum adp_status adp_get(struct adp_pool *,struct adp_handle,uint32_t *);
enum adp_status adp_release(struct adp_pool *,struct adp_handle);
enum adp_status adp_snapshot(struct adp_pool *,struct adp_snapshot *);
/* Payload relocation is serialized with readers; no raw slot pointer escapes. */
enum adp_status adp_compact(struct adp_pool *);
/* Shell relocation requires quiescence of all source users/waiters; initializes
   destination mutex afresh, transfers block ownership, leaves source closed. */
enum adp_status adp_move(struct adp_pool *destination,struct adp_pool *source);
/* Close admits concurrent operations; destroy requires closed + all callers
   joined/quiescent. A closed pool retains a live mutex until destroy. */
enum adp_status adp_close(struct adp_pool *);
enum adp_status adp_destroy(struct adp_pool *);
#endif
