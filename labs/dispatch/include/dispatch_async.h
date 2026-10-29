#ifndef CMASTERY_DISPATCH_ASYNC_H
#define CMASTERY_DISPATCH_ASYNC_H
#include "dispatch.h"
#include <pthread.h>
struct ds_async_registry;

struct ds_async_token
{
    struct ds_async_registry *registry;
    unsigned slot, generation;
};

struct ds_async_entry
{
    struct ds_async_registry *registry;
    struct ds_loaded *owner;
    const struct ds_plugin *api;
    pthread_t thread;
    unsigned generation, live, registration_held, job, go, running, done,
        cancel;
    int state, amount, result, status;
};

struct ds_async_registry
{
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    struct ds_async_entry entries[4];
    unsigned attempts, fail_at, registrations, acknowledgements, invocations,
        initialized;
};

int ds_async_init(struct ds_async_registry *);
int ds_async_destroy(struct ds_async_registry *);
int ds_async_register(struct ds_async_registry *, struct ds_loaded *,
                      struct ds_async_token *);
int ds_async_queue(struct ds_async_token, int, int);
int ds_async_start(struct ds_async_token);
int ds_async_remove(struct ds_async_token);
int ds_async_drain(struct ds_async_token, int *);
typedef int (*ds_collect_gate_fn)(void *);

struct ds_reload
{
    struct ds_loaded modules[2];
    struct d_small states[2];
    struct ds_async_token registrations[2][2];
    struct ds_async_registry *registry;
    unsigned current, retired, generation;
};

void ds_reload_init(struct ds_reload *, struct ds_async_registry *,
                    struct d_allocator);
int ds_reload_change(struct ds_reload *, const char *, const int32_t *, size_t);
int ds_reload_collect(struct ds_reload *, ds_collect_gate_fn, void *);
int ds_reload_stop(struct ds_reload *, ds_collect_gate_fn, void *);
int ds_reload_get(const struct ds_reload *, size_t, int32_t *);
#endif
