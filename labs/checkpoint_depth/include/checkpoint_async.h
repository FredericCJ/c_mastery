#ifndef CHECKPOINT_ASYNC_H
#define CHECKPOINT_ASYNC_H
#include "checkpoint_depth.h"
#include <pthread.h>
#include <time.h>
enum { CPA_SLOTS=4,CPA_QUEUE=2 };
enum cpa_status { CPA_OK,CPA_FULL,CPA_CLOSED,CPA_STALE,CPA_PENDING,CPA_ERROR };
enum cpa_lifecycle { CPA_OPEN,CPA_CLOSING,CPA_DRAINING,CPA_ABORTING,CPA_STOPPED };
struct cpa_handle { unsigned slot,generation; };
struct cpa_slot { struct cp_message message;unsigned generation,state; };
struct cpa_queue {
    pthread_mutex_t mutex;pthread_cond_t changed;
    struct cpa_slot slots[CPA_SLOTS];unsigned queue[CPA_QUEUE];size_t head,count,owned,waiters,wakes;
    size_t accepted,completed,discarded,rejected,releases,report_failures;
    unsigned generation_limit,transitions;enum cpa_lifecycle state;
};
enum cpa_status cpa_init(struct cpa_queue *,unsigned);
enum cpa_status cpa_offer(struct cpa_queue *,const struct cp_message *,int);
enum cpa_status cpa_take(struct cpa_queue *,struct cpa_handle *,struct cp_message *,int);
enum cpa_status cpa_finish(struct cpa_queue *,struct cpa_handle,int);
enum cpa_status cpa_close(struct cpa_queue *,int);
enum cpa_status cpa_wait_closed(struct cpa_queue *,const struct timespec *);
enum cpa_status cpa_destroy(struct cpa_queue *);
#endif
