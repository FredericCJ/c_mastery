#ifndef CMASTERY_OBJECT_RUNTIME_RC_CONCURRENT_H
#define CMASTERY_OBJECT_RUNTIME_RC_CONCURRENT_H
#include "object_runtime_rc.h"
#include <pthread.h>
struct orr_concurrent { pthread_mutex_t mutex; struct orr_heap heap; int initialized; };
enum orr_operation { ORR_NEW, ORR_RETAIN, ORR_RELEASE, ORR_STORE, ORR_FLUSH, ORR_PREPARE, ORR_COMMIT, ORR_INSPECT };
struct orr_request {
  enum orr_operation operation;
  struct orr_handle object,target;
  unsigned field,budget;
  int defer;
  struct orr_trial trial;
};
struct orr_response {
  struct orr_handle object;
  struct orr_trial trial;
  unsigned processed,invariant;
  uint16_t live,dead,revision;
};
enum or_status orrc_init(struct orr_concurrent *);
enum or_status orrc_call(struct orr_concurrent *, const struct orr_request *, struct orr_response *);
enum or_status orrc_close(struct orr_concurrent *);
#endif
