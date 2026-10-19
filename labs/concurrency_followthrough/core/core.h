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
typedef struct {
  uint32_t sequence, inverse;
} CfValue;
#ifndef CF_PAD
#define CF_PAD 1
#endif
typedef struct {
#if CF_PAD
  _Alignas(64)
#endif
      atomic_uint head;
#if CF_PAD
  _Alignas(64)
#endif
      atomic_uint tail;
  unsigned storage, limit, mask;
  CfValue values[32];
} CfRing;
/* Exactly one producer and consumer, no reentry. Hook is bounded test-only,
 * returns normally and never accesses queue/payload or retains arguments. */
typedef void (*CfHook)(void *);
int cf_ring_init(CfRing *, unsigned storage, unsigned limit, unsigned bits);
int cf_push(CfRing *, const CfValue *, unsigned n, CfHook, void *);
int cf_pop(CfRing *, CfValue *, unsigned n, CfHook, void *);

/* Checked counter: injected spuriosity is a test-side weak-CAS implementation
 * contract; actual C weak CAS still executes. Successful calls never wrap. */
int cf_increment(atomic_uint *, unsigned *old_value, unsigned *attempts,
                 unsigned injected_spurious);

enum { CF_FREE, CF_RESERVED, CF_LINKED, CF_RETIRED, CF_RETIRED_FOREVER };
typedef struct {
  atomic_uint state[32];
  unsigned generation_limit;
} CfPool;
typedef struct {
  const CfPool *owner;
  unsigned index, generation;
} CfTicket;
int cf_pool_init(CfPool *, unsigned generation_limit);
int cf_take(CfPool *, unsigned worker, unsigned steal, CfTicket *,
            unsigned *scans);
int cf_publish(CfPool *, CfTicket);
int cf_cancel(CfPool *, CfTicket);
int cf_retire(CfPool *, CfTicket);
/* Sole retirement owner, after an external proof of no possible remaining
 * access. This predicate is a precondition, not a hazard-pointer
 * implementation. */
int cf_reclaim(CfPool *, CfTicket);
#endif
