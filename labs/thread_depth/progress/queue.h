#ifndef THREAD_PROGRESS_QUEUE_H
#define THREAD_PROGRESS_QUEUE_H
#include "waiting.h"

struct tp_end_queue
{
    pthread_mutex_t gate;
    pthread_cond_t quiet;
    struct tw_queue queue;
    unsigned closing;
    unsigned inflight;
    unsigned retired;
    unsigned accepted;
    unsigned delivered;
    unsigned discarded;
    uint64_t discarded_sum;
};

int tp_queue_init(struct tp_end_queue *);
int tp_queue_push(struct tp_end_queue *, int);
int tp_queue_pop(struct tp_end_queue *, int *);
/* Single noncancelable shutdown controller; settle admitted operations, then
   discard the residual copied integers. No new admission after closing. */
int tp_queue_abort(struct tp_end_queue *);
void tp_queue_destroy(struct tp_end_queue *);
#endif
