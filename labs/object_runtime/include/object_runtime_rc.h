#ifndef CMASTERY_OBJECT_RUNTIME_RC_H
#define CMASTERY_OBJECT_RUNTIME_RC_H
#include "object_runtime.h"
struct orr_handle { uint8_t slot, generation; };
struct orr_heap {
  struct or_counts counts;
  uint8_t roots[16], generation[16], pending[8], head, length, closed;
  uint16_t candidates, revision;
};
struct orr_trial {
  const struct orr_heap *owner;
  struct or_counts result;
  uint16_t revision, dead;
};
void orr_init(struct orr_heap *);
enum or_status orr_new(struct orr_heap *, struct orr_handle *);
enum or_status orr_retain(struct orr_heap *, struct orr_handle);
enum or_status orr_release(struct orr_heap *, struct orr_handle, int defer, uint16_t *reclaimed);
enum or_status orr_store(struct orr_heap *, struct orr_handle, unsigned field, struct orr_handle, int defer, uint16_t *reclaimed);
enum or_status orr_flush(struct orr_heap *, unsigned budget, unsigned *processed, uint16_t *reclaimed);
enum or_status orr_prepare(struct orr_heap *, struct orr_trial *);
enum or_status orr_commit(struct orr_heap *, const struct orr_trial *, uint16_t *reclaimed);
enum or_status orr_close(struct orr_heap *);
int orr_invariant(const struct orr_heap *);
#endif
