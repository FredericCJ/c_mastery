#ifndef CMASTERY_CL_POSIX_H
#define CMASTERY_CL_POSIX_H
#include "cl.h"
#include <pthread.h>
#include <time.h>

struct clp_queue
{
    struct cl_queue core;
    struct cl_message slots[2];
    pthread_mutex_t mutex;
    pthread_cond_t readable, writable, enrolled;
    unsigned active, send_waiters, recv_waiters, initialized;
};

/* Fail step 1..4 injects init failure before that POSIX initialization;
 * zero disables injection. Initialized bitmask records cleanup obligations. */
int clp_init(struct clp_queue *, unsigned fail_step);
int clp_send(struct clp_queue *, uint32_t producer, uint32_t sequence,
             const unsigned char *, size_t,
             const struct timespec *absolute_monotonic_deadline);
int clp_recv(struct clp_queue *, unsigned char *, size_t, size_t *, uint32_t *,
             uint32_t *, uint64_t *delivery_ticket,
             const struct timespec *absolute_monotonic_deadline);
int clp_close(struct clp_queue *, unsigned abort_queued);
/* External admission has stopped; caller has joined every accessor. */
int clp_destroy(struct clp_queue *);

/* Linux robust-mutex adapter; lock is initialized by the caller. */
struct clp_record
{
    uint32_t value, checksum, committed;
};

int clp_record_recover(pthread_mutex_t *, struct clp_record *, unsigned repair);
#endif
