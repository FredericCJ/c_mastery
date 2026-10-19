#ifndef CMASTERY_ABSTRACTION_REGISTRY_H
#define CMASTERY_ABSTRACTION_REGISTRY_H
#include "api_depth.h"
typedef struct AsRegistry AsRegistry;
typedef void (*AsCallback)(void *,AsRegistry *,ApiHandle);
typedef void (*AsTerminal)(void *,AsRegistry *);
typedef struct {
 AsRegistry *owner;AsCallback callback;void *context;ApiDrop drop;
 ApiHandle handle;unsigned occupied,enabled;
} AsRegistration;
struct AsRegistry {
 ApiRegistry core;AsRegistration registrations[4];unsigned suppress;
 AsTerminal terminal;void *terminal_context;
};
void as_registry_init(AsRegistry *,unsigned immediate_suppression,AsTerminal,void *);
ApiStatus as_register(AsRegistry *,AsCallback,void *,ApiDrop,ApiHandle *);
ApiStatus as_remove(AsRegistry *,ApiHandle);
ApiStatus as_dispatch(AsRegistry *);
ApiStatus as_destroy(AsRegistry *);
#endif
