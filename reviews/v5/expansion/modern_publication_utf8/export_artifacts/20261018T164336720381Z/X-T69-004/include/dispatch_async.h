#ifndef CMASTERY_DISPATCH_ASYNC_H
#define CMASTERY_DISPATCH_ASYNC_H
#include "dispatch.h"
#include <pthread.h>
typedef struct DSAsyncRegistry DSAsyncRegistry;
typedef struct { DSAsyncRegistry *registry;unsigned slot,generation; } DSAsyncToken;
typedef struct {
 DSAsyncRegistry *registry;DSLoaded *owner;const DSPlugin *api;pthread_t thread;
 unsigned generation,live,registration_held,job,go,running,done,cancel;
 int state,amount,result,status;
} DSAsyncEntry;
struct DSAsyncRegistry {
 pthread_mutex_t mutex;pthread_cond_t condition;DSAsyncEntry entries[4];
 unsigned attempts,fail_at,registrations,acknowledgements,invocations,initialized;
};
int ds_async_init(DSAsyncRegistry *);
int ds_async_destroy(DSAsyncRegistry *);
int ds_async_register(DSAsyncRegistry *,DSLoaded *,DSAsyncToken *);
int ds_async_queue(DSAsyncToken,int,int);
int ds_async_start(DSAsyncToken);
int ds_async_remove(DSAsyncToken);
int ds_async_drain(DSAsyncToken,int *);
typedef int (*DSCollectGate)(void *);
typedef struct {
 DSLoaded modules[2];DSmall states[2];DSAsyncToken registrations[2][2];
 DSAsyncRegistry *registry;unsigned current,retired,generation;
} DSReload;
void ds_reload_init(DSReload *,DSAsyncRegistry *,DAllocator);
int ds_reload_change(DSReload *,const char *,const int32_t *,size_t);
int ds_reload_collect(DSReload *,DSCollectGate,void *);
int ds_reload_stop(DSReload *,DSCollectGate,void *);
int ds_reload_get(const DSReload *,size_t,int32_t *);
#endif
