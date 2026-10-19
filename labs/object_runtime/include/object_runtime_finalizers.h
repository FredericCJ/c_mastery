#ifndef CMASTERY_OBJECT_RUNTIME_FINALIZERS_H
#define CMASTERY_OBJECT_RUNTIME_FINALIZERS_H
#include "object_runtime.h"
typedef struct { uint8_t slot, generation; } ORFHandle;
typedef struct ORFHeap ORFHeap;
typedef ORStatus (*ORFFinalizer)(ORFHeap *, ORFHandle, void *);
struct ORFHeap {
  ORWeak weak;
  ORFFinalizer callback[16];
  void *context[16];
  uint8_t generation[16], busy, closed;
  uint16_t resources, cleanup_pending, callbacks_run, resources_closed;
};
void orf_init(ORFHeap *heap);
ORStatus orf_new(ORFHeap *, ORFFinalizer, void *, int resource, int fail, ORFHandle *);
ORStatus orf_root(ORFHeap *, ORFHandle, int present);
ORStatus orf_edge(ORFHeap *, ORFHandle, unsigned field, ORFHandle);
ORStatus orf_pair(ORFHeap *, unsigned index, ORFHandle key, ORFHandle value);
ORStatus orf_weak(ORFHeap *, unsigned index, ORFHandle);
ORStatus orf_collect(ORFHeap *, uint16_t *reclaimed);
ORStatus orf_run_one(ORFHeap *, ORFHandle *, ORStatus *callback_status);
ORStatus orf_close_resource(ORFHeap *, ORFHandle);
ORStatus orf_cleanup(ORFHeap *, unsigned budget, unsigned *released);
ORStatus orf_close(ORFHeap *);
#endif
