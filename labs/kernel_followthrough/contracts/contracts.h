#ifndef KF_CONTRACTS_H
#define KF_CONTRACTS_H
#include "cmk_contract.h"
typedef struct cmk_boot cmk_boot;
typedef struct cmk_frames cmk_frames;
typedef struct {
  uint64_t start, end;
} KfSpan;
typedef struct {
  cmk_boot core;
  const unsigned char *command;
  size_t command_length;
  size_t visited, next;
} KfBoot;
/* On refusal all caller-visible output bytes remain unchanged. Borrowed spans
 * remain owned by the immutable input. Optional command line must contain NUL.
 */
int kf_boot(const unsigned char *, size_t, KfBoot *);
/* Explicit reservation intervals; full usable pages only, any reserved overlap
 * wins. Unsorted memory records and reservations are allowed. */
int kf_frames(cmk_frames *, const cmk_boot *, const KfSpan *, size_t);
typedef struct {
  uint32_t frame;
  unsigned permissions;
} KfPte;
typedef struct {
  cmk_frames frames;
  unsigned char bytes[256][4096];
} KfPool;
typedef struct {
  KfPte pages[512];
  uint32_t generation;
  unsigned live, retired;
} KfSpace;
typedef struct {
  unsigned current, cr3_owner, tss_owner, stack_owner;
} KfCpu;
size_t kf_free(const KfPool *);
/* Test pool backs exactly [3MiB,4MiB); failure budget counts successful takes.
 * Preflight every existing PTE before allocation. Roll back a partial request.
 */
int kf_allocate(KfPool *, KfSpace *, unsigned first, unsigned count,
                unsigned fail_after);
int kf_reap_user(KfPool *, KfSpace *, unsigned slot, const KfCpu *);
int kf_retire_slot(KfSpace *, unsigned slot, const KfCpu *);
/* Numeric page-entry interpreter: no host address is dereferenced. Permission
 * bits use x86 P/RW/U/NX encodings; returns1 on allowed access,0 otherwise. */
int kf_permission(const uint64_t entry[4], unsigned write, unsigned execute,
                  unsigned user, unsigned *fault);
int kf_next(const unsigned *alive, unsigned count, unsigned current);
int kf_weighted(const unsigned *alive, const unsigned *weight, unsigned count,
                int64_t *credit);
int kf_load_image(KfPool *, KfSpace *, const unsigned char *, size_t,
                  unsigned budget);
int kf_archive_typed(const unsigned char *, size_t, const char *,
                     unsigned expected_type, const unsigned char **, size_t *);
typedef struct {
  uint16_t generation;
  unsigned open, retired, rights, resource;
} KfHandleSlot;
typedef struct {
  KfHandleSlot slot[16];
} KfHandles;
uint32_t kf_handle_open(KfHandles *, unsigned slot, unsigned resource,
                        unsigned rights);
int kf_handle_get(const KfHandles *, uint64_t token, unsigned rights,
                  unsigned *resource);
int kf_handle_close(KfHandles *, uint64_t token);
uint32_t kf_delegate(const KfHandles *, uint64_t, KfHandles *,
                     unsigned destination_slot, unsigned rights);
typedef struct {
  char line[64];
  unsigned used, discard, suppress_lf;
} KfLine;
/*1 completed command,2 overlong discarded record,0 need more; output copied
 * only on completion. CRLF suppression persists across arbitrary chunks. */
int kf_line(KfLine *, unsigned char, char output[64], unsigned *length);
#endif
