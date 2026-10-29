#ifndef CMASTERY_DEADLINE_COLLECTION_H
#define CMASTERY_DEADLINE_COLLECTION_H
#include "timer.h"

enum tc_kind { TC_SCAN, TC_HEAP, TC_WHEEL };
struct tc_token { size_t slot; unsigned generation; };
struct tc_collection {
  struct td_timer *timers;
  size_t *index, *position, *next, *previous, *heads;
  size_t capacity, count, buckets;
  uint64_t quantum, comparisons;
  enum tc_kind kind;
};

/* Caller owns address-stable, nonoverlapping arrays and serializes all calls.
 * Storage lengths: timers/index/position/next/previous = capacity; heads=buckets.
 * Scan requires timers; heap also index/position; wheel also position/next/
 * previous/heads. All other arrays may be null. No allocations or callbacks.
 */
int tc_init(struct tc_collection *, enum tc_kind, struct td_timer *, size_t capacity,
            size_t *index, size_t *position, size_t *next, size_t *previous,
            size_t *heads, size_t buckets, uint64_t quantum, unsigned generation_limit);
int tc_arm(struct tc_collection *, size_t slot, uint64_t deadline, uint64_t period,
           int skip, struct tc_token *);
int tc_cancel(struct tc_collection *, struct tc_token);
int tc_peek(struct tc_collection *, struct tc_token *, uint64_t *deadline);
int tc_begin(struct tc_collection *, uint64_t now, struct tc_token *, struct td_dispatch *);
int tc_finish(struct tc_collection *, struct tc_token);
#endif
