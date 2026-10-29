#ifndef CMASTERY_DEADLINE_COLLECTION_H
#define CMASTERY_DEADLINE_COLLECTION_H
#include "timer.h"

enum TcKind { TC_SCAN, TC_HEAP, TC_WHEEL };
typedef struct { size_t slot; unsigned generation; } TcToken;
typedef struct {
  TdTimer *timers;
  size_t *index, *position, *next, *previous, *heads;
  size_t capacity, count, buckets;
  uint64_t quantum, comparisons;
  enum TcKind kind;
} TcCollection;

/* Caller owns address-stable, nonoverlapping arrays and serializes all calls.
 * Storage lengths: timers/index/position/next/previous = capacity; heads=buckets.
 * Scan requires timers; heap also index/position; wheel also position/next/
 * previous/heads. All other arrays may be null. No allocations or callbacks.
 */
int tc_init(TcCollection *, enum TcKind, TdTimer *, size_t capacity,
            size_t *index, size_t *position, size_t *next, size_t *previous,
            size_t *heads, size_t buckets, uint64_t quantum, unsigned generation_limit);
int tc_arm(TcCollection *, size_t slot, uint64_t deadline, uint64_t period,
           int skip, TcToken *);
int tc_cancel(TcCollection *, TcToken);
int tc_peek(TcCollection *, TcToken *, uint64_t *deadline);
int tc_begin(TcCollection *, uint64_t now, TcToken *, TdDispatch *);
int tc_finish(TcCollection *, TcToken);
#endif
