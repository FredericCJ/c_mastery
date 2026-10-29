#ifndef KF_CONTRACTS_H
#define KF_CONTRACTS_H
#include "cmk_contract.h"
typedef struct cmk_boot cmk_boot;
typedef struct cmk_frames cmk_frames;
struct kf_span {
  uint64_t start, end;
};
struct kf_boot {
  cmk_boot core;
  const unsigned char *command;
  size_t command_length;
  size_t visited, next;
};
/* On refusal all caller-visible output bytes remain unchanged. Borrowed spans
 * remain owned by the immutable input. Optional command line must contain NUL.
 */
int kf_boot(const unsigned char *, size_t, struct kf_boot *);
/* Explicit reservation intervals; full usable pages only, any reserved overlap
 * wins. Unsorted memory records and reservations are allowed. */
int kf_frames(cmk_frames *, const cmk_boot *, const struct kf_span *, size_t);
struct kf_pte {
  uint32_t frame;
  unsigned permissions;
};
struct kf_pool {
  cmk_frames frames;
  unsigned char bytes[256][4096];
};
struct kf_space {
  struct kf_pte pages[512];
  uint32_t generation;
  unsigned live, retired;
};
struct kf_cpu {
  unsigned current, cr3_owner, tss_owner, stack_owner;
};
size_t kf_free(const struct kf_pool *);
/* Test pool backs exactly [3MiB,4MiB); failure budget counts successful takes.
 * Preflight every existing PTE before allocation. Roll back a partial request.
 */
int kf_allocate(struct kf_pool *, struct kf_space *, unsigned first, unsigned count,
                unsigned fail_after);
int kf_reap_user(struct kf_pool *, struct kf_space *, unsigned slot, const struct kf_cpu *);
int kf_retire_slot(struct kf_space *, unsigned slot, const struct kf_cpu *);
/* Numeric page-entry interpreter: no host address is dereferenced. Permission
 * bits use x86 P/RW/U/NX encodings; returns1 on allowed access,0 otherwise. */
int kf_permission(const uint64_t entry[4], unsigned write, unsigned execute,
                  unsigned user, unsigned *fault);
int kf_next(const unsigned *alive, unsigned count, unsigned current);
int kf_weighted(const unsigned *alive, const unsigned *weight, unsigned count,
                int64_t *credit);
int kf_load_image(struct kf_pool *, struct kf_space *, const unsigned char *, size_t,
                  unsigned budget);
int kf_archive_typed(const unsigned char *, size_t, const char *,
                     unsigned expected_type, const unsigned char **, size_t *);
struct kf_handle_slot {
  uint16_t generation;
  unsigned open, retired, rights, resource;
};
struct kf_handles {
  struct kf_handle_slot slot[16];
};
uint32_t kf_handle_open(struct kf_handles *, unsigned slot, unsigned resource,
                        unsigned rights);
int kf_handle_get(const struct kf_handles *, uint64_t token, unsigned rights,
                  unsigned *resource);
int kf_handle_close(struct kf_handles *, uint64_t token);
uint32_t kf_delegate(const struct kf_handles *, uint64_t, struct kf_handles *,
                     unsigned destination_slot, unsigned rights);
struct kf_line {
  char line[64];
  unsigned used, discard, suppress_lf;
};
/*1 completed command,2 overlong discarded record,0 need more; output copied
 * only on completion. CRLF suppression persists across arbitrary chunks. */
int kf_line(struct kf_line *, unsigned char, char output[64], unsigned *length);
#endif
