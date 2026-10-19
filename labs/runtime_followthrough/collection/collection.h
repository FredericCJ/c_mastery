#ifndef CMASTERY_VM_COLLECTION_H
#define CMASTERY_VM_COLLECTION_H
#include "object_runtime_vm.h"
typedef struct Vg Vg;
typedef struct {
  const Vg *owner;
  uint32_t epoch;
  CvmValue value;
} VgRef;
typedef struct {
  const Vg *owner;
  uint32_t epoch;
  uint64_t ticket;
  uint8_t client;
} VgLease;
enum { VG_OK, VG_BAD, VG_STATE, VG_FULL, VG_STALE, VG_DONE, VG_UNIMPLEMENTED };
enum { VG_IDLE, VG_MARK, VG_READY };
struct Vg {
  Cvm vm;
  ORVM registrations;
  ORVDerived roots[16];
  uint8_t clients[16];
  uint64_t tickets[16];
  CvmValue weak[4], temporaries[2];
  uint8_t weak_present[4], temporary_count;
  ORGraph graph;
  uint32_t epoch;
  unsigned phase, inspected, scanned, commits;
};
/* Immovable, caller-owned, zero-initialized controller. All operations
 * serialized; underlying VM is stopped and not mutated except through these
 * operations. Reset refuses active marking/leases; increments an epoch, never
 * wraps. CvmValue alone has no cross-reset identity; VgRef adds live
 * owner+epoch.
 */
int vg_reset(Vg *, unsigned arena);
int vg_bind(Vg *, CvmValue, VgRef *);
int vg_pair(Vg *, VgRef, VgRef, VgRef *);
int vg_field(Vg *, VgRef, unsigned, VgRef *);
int vg_store(Vg *, VgRef, unsigned, VgRef);
/* Two explicit embedding client scopes, backed by actual ORVM/algo_pool
 * tickets. These are ownership checks, not authentication against native C
 * forgery. */
int vg_acquire(Vg *, unsigned client, VgRef, VgLease *);
int vg_release(Vg *, unsigned client, VgLease);
int vg_weak_set(Vg *, unsigned, VgRef);
int vg_weak_acquire(Vg *, unsigned, unsigned client, VgLease *, VgRef *);
int vg_begin(Vg *);
int vg_step(Vg *, unsigned object_budget, unsigned *scanned);
int vg_commit(Vg *, uint16_t *reclaimed);
/* Copy the reachable pair graph into an existing destination arena. Both
 * stopped, different live controllers; no active marking. Source unchanged.
 * Stages entire destination VM; failure preserves destination and result. Fault
 * cut1..3R+1 follows each allocation, each of two field rewrites, and before
 * publication. Zero disables fault injection; cycles/aliases preserved via one
 * forwarding map.
 */
int vg_transfer(Vg *source, Vg *destination, VgRef root, unsigned cut,
                VgRef *out);
#endif
