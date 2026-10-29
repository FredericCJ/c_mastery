#ifndef CMASTERY_OBJECT_RUNTIME_GENERATIONAL_H
#define CMASTERY_OBJECT_RUNTIME_GENERATIONAL_H
#include "object_runtime.h"
enum or_status or_nursery_cards_init(const struct or_nursery *, struct or_cards *);
enum or_status or_nursery_old_store(struct or_nursery *, struct or_cards *, unsigned object,
                              unsigned field, unsigned target);
enum or_status or_minor_preflight(struct or_nursery *, struct or_cards *optional_cards,
                            unsigned young_capacity, unsigned old_capacity,
                            unsigned *copied, unsigned *root_fields_scanned);
enum or_status or_major_preflight(struct or_nursery *, struct or_cards *optional_cards,
                            unsigned *reclaimed, uint8_t forwarding[16]);
struct or_large_handle {
  uint8_t slot, generation;
};
struct or_large_block {
  uint8_t start, length, pins, active, generation;
};
struct or_large_ticket {
  struct or_large_handle object;
  uint8_t active, generation;
};
struct or_large_heap {
  struct or_nursery heap;
  struct or_cards cards;
  uint32_t words[8];
  struct or_large_block block[4];
  struct or_large_ticket ticket[4];
};
struct or_large_lease {
  struct or_large_heap *owner;
  uint8_t slot, generation;
};
void or_large_init(struct or_large_heap *);
enum or_status or_large_allocate(struct or_large_heap *, unsigned length, uint32_t pattern,
                           struct or_large_handle *);
enum or_status or_large_release(struct or_large_heap *, struct or_large_handle);
enum or_status or_large_pin(struct or_large_heap *, struct or_large_handle, struct or_large_lease *,
                      const uint32_t **data, unsigned *length);
enum or_status or_large_unpin(struct or_large_lease);
enum or_status or_large_collect(struct or_large_heap *, unsigned *reclaimed);
#endif
