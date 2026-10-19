#ifndef TS_SHARING_H
#define TS_SHARING_H
#include <pthread.h>
#include <stdint.h>
#include <stddef.h>
int ts_sum(const int32_t *,size_t,unsigned,int,int64_t *);
typedef struct {pthread_mutex_t mutex;int32_t values[16];size_t length;unsigned version;} TSInput;
typedef void (*TSSnapshotHook)(void *);
int ts_snapshot_sum(TSInput *,unsigned,TSSnapshotHook,void *,int64_t *,unsigned *);
enum {TS_CALLER,TS_QUEUED,TS_WORKER,TS_SETTLED};
enum {TS_SUCCESS=1,TS_FAILED,TS_CANCELED};
typedef struct {unsigned state[8],terminal[8],settlements[8],ring[2],head,count,closed;pthread_mutex_t mutex;} TSQueue;
int ts_queue_init(TSQueue *);
int ts_enqueue(TSQueue *,unsigned);
int ts_dequeue(TSQueue *,unsigned *);
int ts_settle(TSQueue *,unsigned,unsigned);
int ts_queue_close(TSQueue *);
int ts_queue_inspect(TSQueue *,unsigned,unsigned *,unsigned *,unsigned *);
int ts_queue_destroy(TSQueue *);
enum {TS_SHARED,TS_PRIVATE,TS_SHARDED,TS_BATCHED};
typedef struct {size_t requested_buffer_bytes;unsigned workers;size_t requested_stack_bytes;} TSResources;
int ts_histogram(const uint32_t *,size_t,size_t,unsigned,size_t,uint64_t *,TSResources *);
#endif
