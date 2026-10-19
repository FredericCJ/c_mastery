#ifndef THREAD_PROGRESS_QUEUE_H
#define THREAD_PROGRESS_QUEUE_H
#include "waiting.h"
typedef struct {
  pthread_mutex_t gate;
  pthread_cond_t quiet;
  TWQueue queue;
  unsigned closing, inflight, retired;
  unsigned accepted, delivered, discarded;
  uint64_t discarded_sum;
} TPEndQueue;
int tp_queue_init(TPEndQueue *);
int tp_queue_push(TPEndQueue *, int);
int tp_queue_pop(TPEndQueue *, int *);
/* Single noncancelable shutdown controller; settle admitted operations, then
   discard the residual copied integers. No new admission after closing. */
int tp_queue_abort(TPEndQueue *);
void tp_queue_destroy(TPEndQueue *);
#endif
