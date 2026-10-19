#ifndef CMASTERY_OBJECT_RUNTIME_RC_H
#define CMASTERY_OBJECT_RUNTIME_RC_H
#include "object_runtime.h"
typedef struct { uint8_t slot, generation; } ORRHandle;
typedef struct {
  ORCounts counts;
  uint8_t roots[16], generation[16], pending[8], head, length, closed;
  uint16_t candidates, revision;
} ORRHeap;
typedef struct {
  const ORRHeap *owner;
  ORCounts result;
  uint16_t revision, dead;
} ORRTrial;
void orr_init(ORRHeap *);
ORStatus orr_new(ORRHeap *, ORRHandle *);
ORStatus orr_retain(ORRHeap *, ORRHandle);
ORStatus orr_release(ORRHeap *, ORRHandle, int defer, uint16_t *reclaimed);
ORStatus orr_store(ORRHeap *, ORRHandle, unsigned field, ORRHandle, int defer, uint16_t *reclaimed);
ORStatus orr_flush(ORRHeap *, unsigned budget, unsigned *processed, uint16_t *reclaimed);
ORStatus orr_prepare(ORRHeap *, ORRTrial *);
ORStatus orr_commit(ORRHeap *, const ORRTrial *, uint16_t *reclaimed);
ORStatus orr_close(ORRHeap *);
int orr_invariant(const ORRHeap *);
#endif
