#ifndef CMASTERY_NOTIFICATIONS_NATIVE_H
#define CMASTERY_NOTIFICATIONS_NATIVE_H
#include "notifications.h"
#include <pthread.h>
typedef struct { CDNHandle recipient; pthread_mutex_t exclusion; unsigned live; } CDNNative;
typedef struct {
  int status, cpu;
  uint32_t count, refused;
  uint64_t fault_sequence, lock_wait_ns, protected_work_ns;
} CDNNObservation;
typedef struct { uint64_t accepted, consumed, cancelled, rejected; uint32_t pending; int invariant; } CDNNState;
int cdnn_init(CDNNative *, CDNHandle);
int cdnn_post(CDNNative *, uint32_t, CDNNObservation *);
int cdnn_take(CDNNative *, uint32_t, CDNNObservation *);
int cdnn_snapshot(CDNNative *, CDNNState *);
int cdnn_destroy(CDNNative *);
#endif
