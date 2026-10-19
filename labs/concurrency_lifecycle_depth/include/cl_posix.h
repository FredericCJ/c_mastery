#ifndef CMASTERY_CL_POSIX_H
#define CMASTERY_CL_POSIX_H
#include "cl.h"
#include <pthread.h>
#include <time.h>
typedef struct {
  CLQueue core;
  CLMessage slots[2];
  pthread_mutex_t mutex;
  pthread_cond_t readable, writable, enrolled;
  unsigned active, send_waiters, recv_waiters, initialized;
} CLPQueue;
/* Fail step 1..4 injects init failure before that POSIX initialization;
 * zero disables injection. Initialized bitmask records cleanup obligations. */
int clp_init(CLPQueue *, unsigned fail_step);
int clp_send(CLPQueue *, uint32_t producer, uint32_t sequence,
             const unsigned char *, size_t,
             const struct timespec *absolute_monotonic_deadline);
int clp_recv(CLPQueue *, unsigned char *, size_t, size_t *, uint32_t *,
             uint32_t *, uint64_t *delivery_ticket,
             const struct timespec *absolute_monotonic_deadline);
int clp_close(CLPQueue *, unsigned abort_queued);
/* External admission has stopped; caller has joined every accessor. */
int clp_destroy(CLPQueue *);
/* Linux robust-mutex adapter; lock is initialized by the caller. */
typedef struct {
  uint32_t value, checksum, committed;
} CLPRecord;
int clp_record_recover(pthread_mutex_t *, CLPRecord *, unsigned repair);
#endif
