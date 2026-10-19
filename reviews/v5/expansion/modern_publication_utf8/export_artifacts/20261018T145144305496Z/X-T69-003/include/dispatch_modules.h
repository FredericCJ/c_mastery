#ifndef CMASTERY_DISPATCH_MODULES_H
#define CMASTERY_DISPATCH_MODULES_H
#include "dispatch.h"
#include "ownership.h"
typedef struct { int (*now)(void *,int *);void *context; } DSClock;
typedef struct { int (*write)(void *,int);void *context; } DSSink;
typedef struct { DSService input;DSClock clock;DSSink sink; } DSModuleServices;
typedef struct {
  DSModule *module;DSModuleServices services;OwShared *read_owner,*io_owner;
  unsigned generation,live;
} DSModuleEntry;
typedef struct DSModuleRegistry { DSModuleEntry entries[2]; } DSModuleRegistry;
typedef struct { DSModuleRegistry *owner;unsigned slot,generation; } DSModuleLease;
void ds_registry_init(DSModuleRegistry *);
int ds_lease_open(DSModuleRegistry *,DSModuleServices,OwShared *,OwShared *,unsigned,DSModuleLease *);
int ds_lease_twice(DSModuleLease,int *);
int ds_lease_stamp(DSModuleLease,int *);
int ds_lease_emit(DSModuleLease,int);
int ds_lease_close(DSModuleLease);
typedef struct { unsigned id; } DSUserKey;
typedef struct { int quota; } DSUserValue;
typedef struct { unsigned slot; } DSMetricKey;
typedef struct { int count; } DSMetricValue;
typedef struct { unsigned live[4];DSUserValue values[4]; } DSUserMap;
typedef struct { unsigned live[4];DSMetricValue values[4]; } DSMetricMap;
int ds_user_put(DSUserMap *,DSUserKey,DSUserValue);
int ds_user_get(const DSUserMap *,DSUserKey,DSUserValue *);
int ds_metric_put(DSMetricMap *,DSMetricKey,DSMetricValue);
int ds_metric_get(const DSMetricMap *,DSMetricKey,DSMetricValue *);
#endif
