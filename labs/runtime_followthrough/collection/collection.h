#ifndef CMASTERY_VM_COLLECTION_H
#define CMASTERY_VM_COLLECTION_H
#include "object_runtime_vm.h"
struct vg_vg;
struct vg_ref {
  const struct vg_vg *owner;
  uint32_t epoch;
  struct cvm_value value;
};
struct vg_lease {
  const struct vg_vg *owner;
  uint32_t epoch;
  uint64_t ticket;
  uint8_t client;
};
enum { VG_OK, VG_BAD, VG_STATE, VG_FULL, VG_STALE, VG_DONE, VG_UNIMPLEMENTED };
enum { VG_IDLE, VG_MARK, VG_READY };
struct vg_vg {
  struct cvm_cvm vm;
  struct or_vm registrations;
  struct orv_derived roots[16];
  uint8_t clients[16];
  uint64_t tickets[16];
  struct cvm_value weak[4], temporaries[2];
  uint8_t weak_present[4], temporary_count;
  struct or_graph graph;
  uint32_t epoch;
  unsigned phase, inspected, scanned, commits;
};
/* Immovable, caller-owned, zero-initialized controller. All operations
 * serialized; underlying VM is stopped and not mutated except through these
 * operations. Reset refuses active marking/leases; increments an epoch, never
 * wraps. CvmValue alone has no cross-reset identity; VgRef adds live
 * owner+epoch.
 */
int vg_reset(struct vg_vg *, unsigned arena);
int vg_bind(struct vg_vg *, struct cvm_value, struct vg_ref *);
int vg_pair(struct vg_vg *, struct vg_ref, struct vg_ref, struct vg_ref *);
int vg_field(struct vg_vg *, struct vg_ref, unsigned, struct vg_ref *);
int vg_store(struct vg_vg *, struct vg_ref, unsigned, struct vg_ref);
/* Two explicit embedding client scopes, backed by actual ORVM/algo_pool
 * tickets. These are ownership checks, not authentication against native C
 * forgery. */
int vg_acquire(struct vg_vg *, unsigned client, struct vg_ref, struct vg_lease *);
int vg_release(struct vg_vg *, unsigned client, struct vg_lease);
int vg_weak_set(struct vg_vg *, unsigned, struct vg_ref);
int vg_weak_acquire(struct vg_vg *, unsigned, unsigned client, struct vg_lease *, struct vg_ref *);
int vg_begin(struct vg_vg *);
int vg_step(struct vg_vg *, unsigned object_budget, unsigned *scanned);
int vg_commit(struct vg_vg *, uint16_t *reclaimed);
/* Copy the reachable pair graph into an existing destination arena. Both
 * stopped, different live controllers; no active marking. Source unchanged.
 * Stages entire destination VM; failure preserves destination and result. Fault
 * cut1..3R+1 follows each allocation, each of two field rewrites, and before
 * publication. Zero disables fault injection; cycles/aliases preserved via one
 * forwarding map.
 */
int vg_transfer(struct vg_vg *source, struct vg_vg *destination, struct vg_ref root, unsigned cut,
                struct vg_ref *out);
#endif
