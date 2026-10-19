#include "waiting.h"
#include <errno.h>
/* Intentionally unfinished. Tests compile before rejecting the missing
 * behavior. */
int tw_bank_init(TWBank *p) {
  (void)p;
  return ENOSYS;
}
int tw_transfer(TWBank *p, int n, TWHook h, void *a) {
  (void)p;
  (void)n;
  (void)h;
  (void)a;
  return ENOSYS;
}
int tw_bank_total(TWBank *p) {
  (void)p;
  return -1;
}
void tw_bank_destroy(TWBank *p) { (void)p; }
void tw_snapshot_init(TWSnapshot *p) { (void)p; }
int tw_snapshot_write(TWSnapshot *p, int n) {
  (void)p;
  (void)n;
  return ENOSYS;
}
int tw_snapshot_read(TWSnapshot *p, int *n) {
  (void)p;
  (void)n;
  return ENOSYS;
}
int tw_key_next(uint64_t *p, uint64_t *q) {
  (void)p;
  (void)q;
  return ENOSYS;
}
int tw_order(uint64_t a, uint64_t b) {
  (void)a;
  (void)b;
  return 0;
}
int tw_queue_init(TWQueue *p, unsigned n, TWInit *r) {
  (void)p;
  (void)n;
  (void)r;
  return ENOSYS;
}
int tw_ref(TWQueue *p) {
  (void)p;
  return ENOSYS;
}
int tw_unref(TWQueue *p) {
  (void)p;
  return ENOSYS;
}
int tw_close(TWQueue *p) {
  (void)p;
  return ENOSYS;
}
int tw_retire(TWQueue *p) {
  (void)p;
  return ENOSYS;
}
void tw_root_destroy(TWQueue *p) { (void)p; }
int tw_push(TWQueue *p, int n, const struct timespec *t) {
  (void)p;
  (void)n;
  (void)t;
  return ENOSYS;
}
int tw_pop(TWQueue *p, int *n, const struct timespec *t, TWHook h, void *a) {
  (void)p;
  (void)n;
  (void)t;
  (void)h;
  (void)a;
  return ENOSYS;
}
int tw_noise(TWQueue *p) {
  (void)p;
  return ENOSYS;
}
int tw_wait_entries(TWQueue *p, unsigned n) {
  (void)p;
  (void)n;
  return ENOSYS;
}
int tw_counts(TWQueue *p, unsigned *r, unsigned *a, size_t *c) {
  (void)p;
  (void)r;
  (void)a;
  (void)c;
  return ENOSYS;
}
int tw_barrier_init(TWBarrier *p, unsigned n) {
  (void)p;
  (void)n;
  return ENOSYS;
}
int tw_barrier_wait(TWBarrier *p) {
  (void)p;
  return ENOSYS;
}
int tw_barrier_break(TWBarrier *p) {
  (void)p;
  return ENOSYS;
}
void tw_barrier_destroy(TWBarrier *p) { (void)p; }
int tw_permits_init(TWPermits *p) {
  (void)p;
  return ENOSYS;
}
int tw_permit_take(TWPermits *p, unsigned *t) {
  (void)p;
  (void)t;
  return ENOSYS;
}
int tw_permit_release(TWPermits *p, unsigned t) {
  (void)p;
  (void)t;
  return ENOSYS;
}
void tw_permits_destroy(TWPermits *p) { (void)p; }
int tw_reader_admit(unsigned r, unsigned w, unsigned n, int p) {
  (void)r;
  (void)w;
  (void)n;
  (void)p;
  return 0;
}
int tw_writer_admit(unsigned r, unsigned w) {
  (void)r;
  (void)w;
  return 0;
}
int tw_select(unsigned n) {
  (void)n;
  return -1;
}
