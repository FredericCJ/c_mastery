#ifndef CMASTERY_OBJECT_RUNTIME_MARKING_H
#define CMASTERY_OBJECT_RUNTIME_MARKING_H
#include "object_runtime.h"
typedef struct {
  uint8_t slot, generation;
} ORCHandle;
typedef enum { ORC_BLACK = 0, ORC_GRAY = 1, ORC_RESTART = 2 } ORCPolicy;
enum { ORC_REV_LIMIT = 65535 };
typedef struct {
  ORGraph graph;
  uint8_t generation[16], roots[16], work[16], used, policy;
  uint32_t revision, scans, restarts, cycles;
} ORCHeap;
typedef struct {
  const ORCHeap *owner;
  uint32_t revision;
  uint16_t white;
} ORCFinish;
void orcm_init(ORCHeap *, ORCPolicy);
int orcm_invariant(const ORCHeap *);
ORStatus orcm_allocate(ORCHeap *, ORCHandle first, ORCHandle second,
                       ORCHandle *out);
ORStatus orcm_retain(ORCHeap *, ORCHandle);
ORStatus orcm_release(ORCHeap *, ORCHandle);
ORStatus orcm_store(ORCHeap *, ORCHandle owner, unsigned field,
                    ORCHandle target);
ORStatus orcm_read(const ORCHeap *, ORCHandle owner, ORCHandle edges[2]);
ORStatus orcm_begin(ORCHeap *);
ORStatus orcm_step(ORCHeap *, unsigned budget, unsigned *scanned);
ORStatus orcm_prepare(const ORCHeap *, ORCFinish *out);
ORStatus orcm_commit(ORCHeap *, ORCFinish, uint16_t *dead);
#endif
