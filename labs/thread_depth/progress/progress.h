#ifndef THREAD_PROGRESS_H
#define THREAD_PROGRESS_H
#include <stddef.h>
#include <stdint.h>
enum TPStatus {
  TP_OK,
  TP_INVALID,
  TP_UNSUPPORTED,
  TP_OVERFLOW,
  TP_NONLINEAR,
  TP_INCOMPLETE
};
/* Priorities0..7, owners-1(no wait) or valid thread index, at most4 threads.
   Recompute from base priorities for withdrawal; never reuse donated output. */
int tp_donate(const unsigned *, const int *, size_t, unsigned *);
int tp_bound(const uint64_t *, size_t, unsigned, uint64_t, uint64_t *);
enum TPEffect {
  TP_ALLOC = 1,
  TP_PAGEFAULT = 2,
  TP_LOG = 4,
  TP_BLOCK = 8,
  TP_PREEMPT = 16,
  TP_UNAUDITED = 32
};
int tp_nonsleepable(unsigned effects);
enum TPOperation { TP_ENQUEUE, TP_DEQUEUE };
enum TPResult { TP_VALUE, TP_EMPTY, TP_FULL };
typedef struct {
  unsigned invocation, response, thread, kind, value, result;
} TPOp;
enum TPPendingPolicy { TP_REQUIRE_COMPLETE, TP_ALLOW_EXTENSION };
typedef struct {
  unsigned length, operation[6], completed_pending_mask, result[6], value[6];
} TPWitness;
/* Finite capacity3 FIFO; at most6 operations. A pending operation is response0;
   its result/value output is unspecified and synthesized if included. */
int tp_history(const TPOp *, size_t, unsigned, TPWitness *);
#endif
