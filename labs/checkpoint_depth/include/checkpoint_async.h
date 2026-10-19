#ifndef CHECKPOINT_ASYNC_H
#define CHECKPOINT_ASYNC_H
#include "checkpoint_depth.h"
#include <pthread.h>
#include <time.h>
enum { CPA_SLOTS=4,CPA_QUEUE=2 };
typedef enum { CPA_OK,CPA_FULL,CPA_CLOSED,CPA_STALE,CPA_PENDING,CPA_ERROR } CPAStatus;
typedef enum { CPA_OPEN,CPA_CLOSING,CPA_DRAINING,CPA_ABORTING,CPA_STOPPED } CPALifecycle;
typedef struct { unsigned slot,generation; } CPAHandle;
typedef struct { CPMessage message;unsigned generation,state; } CPASlot;
typedef struct {
    pthread_mutex_t mutex;pthread_cond_t changed;
    CPASlot slots[CPA_SLOTS];unsigned queue[CPA_QUEUE];size_t head,count,owned,waiters,wakes;
    size_t accepted,completed,discarded,rejected,releases,report_failures;
    unsigned generation_limit,transitions;CPALifecycle state;
} CPAQueue;
CPAStatus cpa_init(CPAQueue *,unsigned);
CPAStatus cpa_offer(CPAQueue *,const CPMessage *,int);
CPAStatus cpa_take(CPAQueue *,CPAHandle *,CPMessage *,int);
CPAStatus cpa_finish(CPAQueue *,CPAHandle,int);
CPAStatus cpa_close(CPAQueue *,int);
CPAStatus cpa_wait_closed(CPAQueue *,const struct timespec *);
CPAStatus cpa_destroy(CPAQueue *);
#endif
