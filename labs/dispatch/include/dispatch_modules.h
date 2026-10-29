#ifndef CMASTERY_DISPATCH_MODULES_H
#define CMASTERY_DISPATCH_MODULES_H
#include "dispatch.h"
#include "ownership.h"
struct ds_clock { int (*now)(void *,int *);void *context; };
struct ds_sink { int (*write)(void *,int);void *context; };
struct ds_module_services { struct ds_service input;struct ds_clock clock;struct ds_sink sink; };
struct ds_module_entry {
  struct ds_module *module;struct ds_module_services services;struct ow_shared *read_owner,*io_owner;
  unsigned generation,live;
};
struct ds_module_registry { struct ds_module_entry entries[2]; };
struct ds_module_lease { struct ds_module_registry *owner;unsigned slot,generation; };
void ds_registry_init(struct ds_module_registry *);
int ds_lease_open(struct ds_module_registry *,struct ds_module_services,struct ow_shared *,struct ow_shared *,unsigned,struct ds_module_lease *);
int ds_lease_twice(struct ds_module_lease,int *);
int ds_lease_stamp(struct ds_module_lease,int *);
int ds_lease_emit(struct ds_module_lease,int);
int ds_lease_close(struct ds_module_lease);
struct ds_user_key { unsigned id; };
struct ds_user_value { int quota; };
struct ds_metric_key { unsigned slot; };
struct ds_metric_value { int count; };
struct ds_user_map { unsigned live[4];struct ds_user_value values[4]; };
struct ds_metric_map { unsigned live[4];struct ds_metric_value values[4]; };
int ds_user_put(struct ds_user_map *,struct ds_user_key,struct ds_user_value);
int ds_user_get(const struct ds_user_map *,struct ds_user_key,struct ds_user_value *);
int ds_metric_put(struct ds_metric_map *,struct ds_metric_key,struct ds_metric_value);
int ds_metric_get(const struct ds_metric_map *,struct ds_metric_key,struct ds_metric_value *);
#endif
