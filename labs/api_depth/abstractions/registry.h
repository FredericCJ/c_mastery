#ifndef CMASTERY_ABSTRACTION_REGISTRY_H
#define CMASTERY_ABSTRACTION_REGISTRY_H
#include "api_depth.h"
struct as_registry;
typedef void (*as_callback_fn)(void *, struct as_registry *, struct api_handle);
typedef void (*as_terminal_fn)(void *, struct as_registry *);

struct as_registration
{
    struct as_registry *owner;
    as_callback_fn callback;
    void *context;
    api_drop_fn drop;
    struct api_handle handle;
    unsigned occupied, enabled;
};

struct as_registry
{
    struct api_registry core;
    struct as_registration registrations[4];
    unsigned suppress;
    as_terminal_fn terminal;
    void *terminal_context;
};

void as_registry_init(struct as_registry *, unsigned immediate_suppression,
                      as_terminal_fn, void *);
enum api_status as_register(struct as_registry *, as_callback_fn, void *,
                            api_drop_fn, struct api_handle *);
enum api_status as_remove(struct as_registry *, struct api_handle);
enum api_status as_dispatch(struct as_registry *);
enum api_status as_destroy(struct as_registry *);
#endif
