#ifndef CMASTERY_BARRIER_NATIVE_H
#define CMASTERY_BARRIER_NATIVE_H
#include "barrier.h"
#include <pthread.h>

struct cdb_native
{
    cmx_kernel kernel;
    struct cd_barrier barrier;
    struct cdb_member member[4];
    pthread_mutex_t exclusion;
    pthread_cond_t changed;
    unsigned parties, live;
    uint64_t arrivals;
};

struct cdb_tree
{
    struct cdb_native leaf[2], root;
    unsigned live;
};

int cdbn_init(struct cdb_native *, unsigned parties);
int cdbn_wait(struct cdb_native *, unsigned participant);
int cdbn_destroy(struct cdb_native *);
int cdbt_init(struct cdb_tree *);
int cdbt_wait(struct cdb_tree *, unsigned participant);
int cdbt_destroy(struct cdb_tree *);
#endif
