#ifndef CMASTERY_DISPATCH_DEFERRED_H
#define CMASTERY_DISPATCH_DEFERRED_H
#include "dispatch.h"
#include "ownership.h"
#include <pthread.h>
typedef int (*DSDeferredBody)(void *,int,int *);
typedef struct DSDeferred DSDeferred;
typedef struct {DSDeferred *owner;unsigned generation;} DSDeferredToken;
struct DSDeferred {
 pthread_mutex_t mutex;pthread_cond_t condition;pthread_t thread;
 OwShared *environment;DSDeferredBody body;
 unsigned generation,initialized,active,job,go,cancel,running,done;
 int argument,result,status;
};
int ds_deferred_init(DSDeferred *);
int ds_deferred_destroy(DSDeferred *);
int ds_deferred_bind(DSDeferred *,OwShared *,DSDeferredBody,DSDeferredToken *);
int ds_deferred_submit(DSDeferredToken,int);
int ds_deferred_start(DSDeferredToken);
int ds_deferred_cancel(DSDeferredToken);
int ds_deferred_drain(DSDeferredToken,int *);
#endif
