#include "dispatch_modules.h"
void ds_registry_init(DSModuleRegistry *r){(void)r;}
int ds_lease_open(DSModuleRegistry *r,DSModuleServices s,OwShared *a,OwShared *b,unsigned n,DSModuleLease *o){(void)r;(void)s;(void)a;(void)b;(void)n;(void)o;return DS_FAILED;}
int ds_lease_twice(DSModuleLease l,int *o){(void)l;(void)o;return DS_FAILED;}
int ds_lease_stamp(DSModuleLease l,int *o){(void)l;(void)o;return DS_FAILED;}
int ds_lease_emit(DSModuleLease l,int v){(void)l;(void)v;return DS_FAILED;}
int ds_lease_close(DSModuleLease l){(void)l;return DS_FAILED;}
int ds_user_put(DSUserMap *m,DSUserKey k,DSUserValue v){(void)m;(void)k;(void)v;return DS_FAILED;}
int ds_user_get(const DSUserMap *m,DSUserKey k,DSUserValue *o){(void)m;(void)k;(void)o;return DS_FAILED;}
int ds_metric_put(DSMetricMap *m,DSMetricKey k,DSMetricValue v){(void)m;(void)k;(void)v;return DS_FAILED;}
int ds_metric_get(const DSMetricMap *m,DSMetricKey k,DSMetricValue *o){(void)m;(void)k;(void)o;return DS_FAILED;}
