#ifndef CMASTERY_COMPLETION_DEPTH_H
#define CMASTERY_COMPLETION_DEPTH_H
#include <stddef.h>
#include <stdint.h>
enum CoState { CO_FREE, CO_BUSY, CO_DONE, CO_RETIRED };
typedef struct {
  unsigned generation;
  enum CoState state;
  uint64_t token;
  unsigned char *buffer;
  size_t capacity, requested;
  int result, cancel_pending, cancel_seen, cancel_result;
} CoSlot;
typedef struct {
  CoSlot slot[4];
  unsigned generation_limit;
} CoPool;
/* Serialized single issuer. Buffers remain owned by the pool throughout its
 * lifetime. A successful target CQE permits inspection; release permits reuse.
 * A cancel CQE never ends the target buffer's loan. */
int co_init(CoPool *, unsigned char *buffers[4], size_t capacity,
            unsigned limit);
int co_acquire(CoPool *, unsigned slot, size_t requested, uint64_t *token);
int co_cancel(CoPool *, uint64_t target, uint64_t *cancel_token);
int co_complete(CoPool *, uint64_t token, int result, unsigned cqe_flags);
int co_release(CoPool *, uint64_t target);
int co_quiescent(const CoPool *);
#endif
