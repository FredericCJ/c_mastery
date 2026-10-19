#ifndef TL_LIFECYCLE_H
#define TL_LIFECYCLE_H
#include <pthread.h>
#include <stddef.h>
typedef struct TLPool TLPool;
typedef struct { unsigned created, joined, completed, sum; int create_error; } TLStats;
typedef int (*TLJoin)(pthread_t, void **, void *);
TLPool *tl_pool_start(unsigned workers, unsigned fail_ordinal, int fail_allocation, int *error);
int tl_pool_collect(TLPool *, TLJoin, void *, TLStats *);
int tl_pool_dispose(TLPool **);
typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t changed;
    unsigned created, creator_waiting, release_worker, completed;
    unsigned live_refs, freed, sum;
} TLMonitor;
typedef struct {
    TLMonitor *monitor;
    const unsigned char *input;
    size_t length;
    int wait_for_completion;
    int error;
} TLCreator;
int tl_monitor_init(TLMonitor *);
int tl_monitor_destroy(TLMonitor *);
void *tl_detached_creator(void *);
#endif
