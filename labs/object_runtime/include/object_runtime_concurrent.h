#ifndef CMASTERY_OBJECT_RUNTIME_CONCURRENT_H
#define CMASTERY_OBJECT_RUNTIME_CONCURRENT_H
#include "object_runtime_marking.h"
#include <pthread.h>
struct orc_concurrent {
  pthread_mutex_t mutex;
  struct orc_heap heap;
  int initialized;
};
enum orc_operation {
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
};
struct orc_request {
  enum orc_operation operation;
  struct orc_handle object, target, second;
  unsigned field, budget;
  struct orc_finish finish;
};
struct orc_response {
  struct orc_handle object, edge[2];
  struct orc_finish finish;
  unsigned scanned, work, invariant;
  uint16_t live, roots, dead;
  uint32_t revision;
};
enum or_status orcm_concurrent_init(struct orc_concurrent *, enum orc_policy);
enum or_status orcm_concurrent_call(struct orc_concurrent *, const struct orc_request *,
                              struct orc_response *);
enum or_status orcm_concurrent_close(struct orc_concurrent *);
#endif
