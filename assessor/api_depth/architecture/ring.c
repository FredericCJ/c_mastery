#include "ring.h"
#ifndef AR_MUTANT
#define AR_MUTANT 0
#endif
int ar_ring_init(ArRingPolicy *r, size_t capacity) {
  if (!r || capacity > AR_CAP)
    return AR_INVALID;
  *r = (ArRingPolicy){0};
  return cd_ring_init(&r->ring, r->indices, capacity) == CD_OK ? AR_OK
                                                               : AR_INVALID;
}
int ar_ring_push(ArRingPolicy *r, ArItem value, unsigned policy,
                 ArEviction *out) {
  if (!r || !out || policy > 2)
    return AR_INVALID;
  CDSpans spans;
  if (cd_ring_read(&r->ring, &spans) != CD_OK)
    return AR_STATE;
  size_t count = r->ring.count;
  if (!r->ring.capacity)
    return AR_FULL;
  ArEviction e = {0};
  if (count == r->ring.capacity) {
    if (!policy)
      return AR_FULL;
    int slots[AR_CAP];
    for (size_t i = 0; i < count; ++i)
      slots[i] = i < spans.first_count ? spans.first[i]
                                       : spans.second[i - spans.first_count];
    size_t victim = 0;
    if (policy == 2) {
      for (size_t i = 1; i < count; ++i)
        if (r->values[slots[i]].priority < r->values[slots[victim]].priority)
          victim = i;
      if (value.priority <= r->values[slots[victim]].priority)
        return AR_FULL;
    }
    e = (ArEviction){1, r->values[slots[victim]]};
    int discarded[AR_CAP];
    if (cd_ring_pop(&r->ring, discarded, count) != CD_OK)
      return AR_STATE;
    r->live[slots[victim]] = 0;
    for (size_t i = 0; i < count; ++i)
      if (i != victim) {
        size_t wrote;
        if (cd_ring_push(&r->ring, slots + i, 1, 0, &wrote) != CD_OK)
          return AR_STATE;
      }
  }
  int index = -1;
  for (unsigned i = 0; i < AR_CAP; ++i)
    if (!r->live[i]) {
      index = (int)i;
      break;
    }
  if (index < 0)
    return AR_STATE;
  r->live[index] = 1;
  r->values[index] = value;
  size_t wrote;
  if (cd_ring_push(&r->ring, &index, 1, 0, &wrote) != CD_OK)
    return AR_STATE;
  if (AR_MUTANT == 11)
    e.valid = 0;
  *out = e;
  return AR_OK;
}
int ar_ring_pop(ArRingPolicy *r, ArItem *out) {
  if (!r || !out)
    return AR_INVALID;
  int index;
  enum CDStatus s = cd_ring_pop(&r->ring, &index, 1);
  if (s != CD_OK)
    return s == CD_EMPTY ? AR_EMPTY : AR_STATE;
  *out = r->values[index];
  r->live[index] = 0;
  return AR_OK;
}
