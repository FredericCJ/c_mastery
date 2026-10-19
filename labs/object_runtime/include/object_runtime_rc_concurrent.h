#ifndef CMASTERY_OBJECT_RUNTIME_RC_CONCURRENT_H
#define CMASTERY_OBJECT_RUNTIME_RC_CONCURRENT_H
#include "object_runtime_rc.h"
#include <pthread.h>
typedef struct { pthread_mutex_t mutex; ORRHeap heap; int initialized; } ORRConcurrent;
typedef enum { ORR_NEW, ORR_RETAIN, ORR_RELEASE, ORR_STORE, ORR_FLUSH, ORR_PREPARE, ORR_COMMIT, ORR_INSPECT } ORROperation;
typedef struct {
  ORROperation operation;
  ORRHandle object,target;
  unsigned field,budget;
  int defer;
  ORRTrial trial;
} ORRRequest;
typedef struct {
  ORRHandle object;
  ORRTrial trial;
  unsigned processed,invariant;
  uint16_t live,dead,revision;
} ORRResponse;
ORStatus orrc_init(ORRConcurrent *);
ORStatus orrc_call(ORRConcurrent *, const ORRRequest *, ORRResponse *);
ORStatus orrc_close(ORRConcurrent *);
#endif
