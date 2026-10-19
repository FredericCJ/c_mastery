#ifndef STORAGE_FOLLOW_READERS_H
#define STORAGE_FOLLOW_READERS_H
#include "cache.h"
#include "lookup.h"
#include <pthread.h>
enum {SR_OK=0,SR_BUSY=20,SR_STALE=21,SR_CLOSED=22};
typedef struct {SlSnapshot snapshot;uint64_t serial;int live;} SrSlot;
typedef struct SrHub {pthread_mutex_t mutex;store writer,reader;store_io backend;ScCache cache;SrSlot slots[2];uint64_t identity,revision;int closing,poisoned,initialized;} SrHub;
typedef struct {SrHub *owner;unsigned slot;uint64_t serial;} SrLease;
/* Fresh, stable, unowned control; existing exclusive image, not formatting.
 * Underlying IO callbacks cannot reenter, retain buffers or call hub methods.
 * One process owns the backend through this hub; no raw store/backend mutation.
 * Both handle operations and the shared cache are protected by this mutex. */
int sr_init(SrHub *,store_io,uint64_t identity);
int sr_publish(SrHub *,const void *key,size_t,const void *value,size_t);
int sr_get(SrHub *,const void *key,size_t,void *value,size_t,size_t *length);
/* Leases own a fixed copied version. Output initially owns no lease. No forged,
 * modified or ownership-duplicated tokens; owner remains live. Read outputs
 * are disjoint from controls, keys, lease and snapshot storage. */
int sr_capture(SrHub *,SrLease *);
int sr_snapshot_get(SrLease,const void *key,size_t,void *value,size_t,size_t *length);
int sr_release(SrLease);
/* Close always refuses new work, but permits retained snapshot reads/releases.
 * BUSY means slots still owned. Destroy requires close OK and joined callers. */
int sr_close(SrHub *);
int sr_destroy(SrHub *);
#endif
