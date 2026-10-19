/* Unsolved learner implementation: intentionally rejects semantic tests. */
#define _POSIX_C_SOURCE 200809L
#include "cl_posix.h"
int clp_init(CLPQueue *q, unsigned fail) {
  (void)q;
  (void)fail;
  return 1;
}
int clp_send(CLPQueue *q, uint32_t p, uint32_t seq, const unsigned char *in,
             size_t n, const struct timespec *deadline) {
  (void)q;
  (void)p;
  (void)seq;
  (void)in;
  (void)n;
  (void)deadline;
  return 1;
}
int clp_recv(CLPQueue *q, unsigned char *out, size_t cap, size_t *n,
             uint32_t *p, uint32_t *seq, uint64_t *ticket,
             const struct timespec *deadline) {
  (void)q;
  (void)out;
  (void)cap;
  (void)n;
  (void)p;
  (void)seq;
  (void)ticket;
  (void)deadline;
  return 1;
}
int clp_close(CLPQueue *q, unsigned abort_queued) {
  (void)q;
  (void)abort_queued;
  return 1;
}
int clp_destroy(CLPQueue *q) {
  (void)q;
  return 1;
}
int clp_record_recover(pthread_mutex_t *m, CLPRecord *r, unsigned repair) {
  (void)m;
  (void)r;
  (void)repair;
  return 1;
}
