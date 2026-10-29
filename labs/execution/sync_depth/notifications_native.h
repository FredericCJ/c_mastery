#ifndef CMASTERY_NOTIFICATIONS_NATIVE_H
#define CMASTERY_NOTIFICATIONS_NATIVE_H
#include "notifications.h"
#include <pthread.h>

struct cdn_native
{
    struct cdn_handle recipient;
    pthread_mutex_t exclusion;
    unsigned live;
};

struct cdnn_observation
{
    int status, cpu;
    uint32_t count, refused;
    uint64_t fault_sequence, lock_wait_ns, protected_work_ns;
};

struct cdnn_state
{
    uint64_t accepted, consumed, cancelled, rejected;
    uint32_t pending;
    int invariant;
};

int cdnn_init(struct cdn_native *, struct cdn_handle);
int cdnn_post(struct cdn_native *, uint32_t, struct cdnn_observation *);
int cdnn_take(struct cdn_native *, uint32_t, struct cdnn_observation *);
int cdnn_snapshot(struct cdn_native *, struct cdnn_state *);
int cdnn_destroy(struct cdn_native *);
#endif
