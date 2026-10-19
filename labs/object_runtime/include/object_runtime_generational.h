#ifndef CMASTERY_OBJECT_RUNTIME_GENERATIONAL_H
#define CMASTERY_OBJECT_RUNTIME_GENERATIONAL_H
#include "object_runtime.h"
ORStatus or_nursery_cards_init(const ORNursery *, ORCards *);
ORStatus or_nursery_old_store(ORNursery *, ORCards *, unsigned object,
                              unsigned field, unsigned target);
ORStatus or_minor_preflight(ORNursery *, ORCards *optional_cards,
                            unsigned young_capacity, unsigned old_capacity,
                            unsigned *copied, unsigned *root_fields_scanned);
ORStatus or_major_preflight(ORNursery *, ORCards *optional_cards,
                            unsigned *reclaimed, uint8_t forwarding[16]);
typedef struct {
  uint8_t slot, generation;
} ORLargeHandle;
typedef struct {
  uint8_t start, length, pins, active, generation;
} ORLargeBlock;
typedef struct {
  ORLargeHandle object;
  uint8_t active, generation;
} ORLargeTicket;
typedef struct {
  ORNursery heap;
  ORCards cards;
  uint32_t words[8];
  ORLargeBlock block[4];
  ORLargeTicket ticket[4];
} ORLargeHeap;
typedef struct {
  ORLargeHeap *owner;
  uint8_t slot, generation;
} ORLargeLease;
void or_large_init(ORLargeHeap *);
ORStatus or_large_allocate(ORLargeHeap *, unsigned length, uint32_t pattern,
                           ORLargeHandle *);
ORStatus or_large_release(ORLargeHeap *, ORLargeHandle);
ORStatus or_large_pin(ORLargeHeap *, ORLargeHandle, ORLargeLease *,
                      const uint32_t **data, unsigned *length);
ORStatus or_large_unpin(ORLargeLease);
ORStatus or_large_collect(ORLargeHeap *, unsigned *reclaimed);
#endif
