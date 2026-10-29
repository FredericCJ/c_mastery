#ifndef CMASTERY_DISPATCH_DEFERRED_H
#define CMASTERY_DISPATCH_DEFERRED_H
#include "dispatch.h"
#include "ownership.h"
#include <pthread.h>
typedef int (*ds_deferred_body_fn)(void *, int, int *);
struct ds_deferred;

struct ds_deferred_token
{
    struct ds_deferred *owner;
    unsigned generation;
};

struct ds_deferred
{
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    pthread_t thread;
    struct ow_shared *environment;
    ds_deferred_body_fn body;
    unsigned generation, initialized, active, job, go, cancel, running, done;
    int argument, result, status;
};

int ds_deferred_init(struct ds_deferred *);
int ds_deferred_destroy(struct ds_deferred *);
int ds_deferred_bind(struct ds_deferred *, struct ow_shared *,
                     ds_deferred_body_fn, struct ds_deferred_token *);
int ds_deferred_submit(struct ds_deferred_token, int);
int ds_deferred_start(struct ds_deferred_token);
int ds_deferred_cancel(struct ds_deferred_token);
int ds_deferred_drain(struct ds_deferred_token, int *);
#endif
