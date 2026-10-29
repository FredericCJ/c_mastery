#ifndef CMASTERY_CONTRACT_MODES_H
#define CMASTERY_CONTRACT_MODES_H
#include "errors.h"
#include <threads.h>
#ifdef ECM_POSIX_THREADS
#include <pthread.h>
typedef pthread_mutex_t ecm_mutex;
#else
typedef mtx_t ecm_mutex;
#endif
struct ecm_buffer
{
    unsigned char *data;
    size_t length, reports;
};

struct ecm_shared
{
    ecm_mutex mutex;
    struct ecm_buffer buffer;
    int counter;
};

struct ecm_snapshot
{
    int counter;
    size_t reports, length;
    unsigned char bytes[8];
};

er_code ecm_init(struct ecm_buffer *, unsigned char *, size_t length,
                 size_t reports);
er_code ecm_write(struct ecm_buffer *, er_mode, size_t index,
                  unsigned char value);
/* Fresh noncopied control; borrowed data lives until quiescent joined destroy.
   Lock acquisition failure returns IO without effects. Unexpected unlock failure
   terminates the original local process: an already committed effect is not
   reported as an unchanged-output operational failure. No reentry/cancellation. */
er_code ecm_shared_init(struct ecm_shared *, unsigned char *, size_t, int,
                        size_t);
er_code ecm_shared_write(struct ecm_shared *, er_mode, size_t, unsigned char);
er_code ecm_shared_counter(struct ecm_shared *, er_mode, int);
er_code ecm_shared_snapshot(struct ecm_shared *, struct ecm_snapshot *);
void ecm_shared_destroy(struct ecm_shared *);
#endif
