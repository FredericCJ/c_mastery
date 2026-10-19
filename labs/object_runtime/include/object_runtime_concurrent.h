#ifndef CMASTERY_OBJECT_RUNTIME_CONCURRENT_H
#define CMASTERY_OBJECT_RUNTIME_CONCURRENT_H
#include "object_runtime_marking.h"
#include <pthread.h>
typedef struct {
  pthread_mutex_t mutex;
  ORCHeap heap;
  int initialized;
} ORCConcurrent;
typedef enum {
  ORC_ALLOCATE,
  ORC_RETAIN,
  ORC_RELEASE,
  ORC_STORE,
  ORC_READ,
  ORC_BEGIN,
  ORC_STEP,
  ORC_PREPARE,
  ORC_COMMIT,
  ORC_INSPECT
} ORCOperation;
typedef struct {
  ORCOperation operation;
  ORCHandle object, target, second;
  unsigned field, budget;
  ORCFinish finish;
} ORCRequest;
typedef struct {
  ORCHandle object, edge[2];
  ORCFinish finish;
  unsigned scanned, work, invariant;
  uint16_t live, roots, dead;
  uint32_t revision;
} ORCResponse;
ORStatus orcm_concurrent_init(ORCConcurrent *, ORCPolicy);
ORStatus orcm_concurrent_call(ORCConcurrent *, const ORCRequest *,
                              ORCResponse *);
ORStatus orcm_concurrent_close(ORCConcurrent *);
#endif
