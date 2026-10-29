#include "arch.h"
#ifndef AR_MUTANT
#define AR_MUTANT 0
#endif
int ar_queue_init(struct ar_queue *q, size_t cap) {
  if (!q || cap > AR_CAP)
    return AR_INVALID;
  *q = (struct ar_queue){.capacity = cap};
  return AR_OK;
}
int ar_push(struct ar_queue *q, struct ar_item value, unsigned policy, struct ar_eviction *out) {
  if (!q || !out || policy > 2 || q->capacity > AR_CAP ||
      q->length > q->capacity)
    return AR_INVALID;
  if (!q->capacity)
    return AR_FULL;
  struct ar_eviction e = {0};
  if (q->length == q->capacity) {
    if (policy == 0)
      return AR_FULL;
    size_t at = 0;
    if (policy == 2) {
      for (size_t i = 1; i < q->length; ++i)
        if (q->items[i].priority < q->items[at].priority)
          at = i;
      if (value.priority <= q->items[at].priority)
        return AR_FULL;
    }
    e = (struct ar_eviction){1, q->items[at]};
    for (size_t i = at; i + 1 < q->length; ++i)
      q->items[i] = q->items[i + 1];
    --q->length;
  }
  q->items[q->length++] = value;
  if (AR_MUTANT == 2)
    e.valid = 0;
  *out = e;
  return AR_OK;
}
int ar_pop(struct ar_queue *q, struct ar_item *out) {
  if (!q || !out || q->length > q->capacity)
    return AR_INVALID;
  if (!q->length)
    return AR_EMPTY;
  *out = q->items[0];
  for (size_t i = 1; i < q->length; ++i)
    q->items[i - 1] = q->items[i];
  --q->length;
  return AR_OK;
}
