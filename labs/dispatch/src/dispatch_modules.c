#include "dispatch_modules.h"
void ds_registry_init(struct ds_module_registry *r){(void)r;}
int ds_lease_open(struct ds_module_registry *r,struct ds_module_services s,struct ow_shared *a,struct ow_shared *b,unsigned n,struct ds_module_lease *o){(void)r;(void)s;(void)a;(void)b;(void)n;(void)o;return DS_FAILED;}
int ds_lease_twice(struct ds_module_lease l,int *o){(void)l;(void)o;return DS_FAILED;}
int ds_lease_stamp(struct ds_module_lease l,int *o){(void)l;(void)o;return DS_FAILED;}
int ds_lease_emit(struct ds_module_lease l,int v){(void)l;(void)v;return DS_FAILED;}
int ds_lease_close(struct ds_module_lease l){(void)l;return DS_FAILED;}
int ds_user_put(struct ds_user_map *m,struct ds_user_key k,struct ds_user_value v){(void)m;(void)k;(void)v;return DS_FAILED;}
int ds_user_get(const struct ds_user_map *m,struct ds_user_key k,struct ds_user_value *o){(void)m;(void)k;(void)o;return DS_FAILED;}
int ds_metric_put(struct ds_metric_map *m,struct ds_metric_key k,struct ds_metric_value v){(void)m;(void)k;(void)v;return DS_FAILED;}
int ds_metric_get(const struct ds_metric_map *m,struct ds_metric_key k,struct ds_metric_value *o){(void)m;(void)k;(void)o;return DS_FAILED;}
