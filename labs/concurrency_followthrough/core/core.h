#ifndef CF_CORE_H
#define CF_CORE_H
#include <limits.h>
#include <stdatomic.h>
#include <stdint.h>
_Static_assert(UINT_MAX == UINT32_MAX, "selected unsigned width");
enum {
  CF_OK,
  CF_FULL,
  CF_EMPTY,
  CF_INVALID,
  CF_STALE,
  CF_EXHAUSTED,
  CF_BUSY,
  CF_UNIMPLEMENTED = 99
};
struct cf_value {
  uint32_t sequence, inverse;
};
#ifndef CF_PAD
#define CF_PAD 1
#endif
struct cf_ring {
#if CF_PAD
  _Alignas(64)
#endif
      atomic_uint head;
#if CF_PAD
  _Alignas(64)
#endif
      atomic_uint tail;
  unsigned storage, limit, mask;
  struct cf_value values[32];
};
/* Exactly one producer and consumer, no reentry. Hook is bounded test-only,
 * returns normally and never accesses queue/payload or retains arguments. */
typedef void (*cf_hook_fn)(void *);
int cf_ring_init(struct cf_ring *, unsigned storage, unsigned limit, unsigned bits);
int cf_push(struct cf_ring *, const struct cf_value *, unsigned n, cf_hook_fn, void *);
int cf_pop(struct cf_ring *, struct cf_value *, unsigned n, cf_hook_fn, void *);

/* Checked counter: injected spuriosity is a test-side weak-CAS implementation
 * contract; actual C weak CAS still executes. Successful calls never wrap. */
int cf_increment(atomic_uint *, unsigned *old_value, unsigned *attempts,
                 unsigned injected_spurious);

enum { CF_FREE, CF_RESERVED, CF_LINKED, CF_RETIRED, CF_RETIRED_FOREVER };
struct cf_pool {
  atomic_uint state[32];
  unsigned generation_limit;
};
struct cf_ticket {
  const struct cf_pool *owner;
  unsigned index, generation;
};
int cf_pool_init(struct cf_pool *, unsigned generation_limit);
int cf_take(struct cf_pool *, unsigned worker, unsigned steal, struct cf_ticket *,
            unsigned *scans);
int cf_publish(struct cf_pool *, struct cf_ticket);
int cf_cancel(struct cf_pool *, struct cf_ticket);
int cf_retire(struct cf_pool *, struct cf_ticket);
/* Sole retirement owner, after an external proof of no possible remaining
 * access. This predicate is a precondition, not a hazard-pointer
 * implementation. */
int cf_reclaim(struct cf_pool *, struct cf_ticket);
#endif
