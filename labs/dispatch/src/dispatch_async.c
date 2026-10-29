#include "dispatch_async.h"
int ds_async_init(struct ds_async_registry *r){(void)r;return DS_FAILED;}
int ds_async_destroy(struct ds_async_registry *r){(void)r;return DS_FAILED;}
int ds_async_register(struct ds_async_registry *r,struct ds_loaded *m,struct ds_async_token *t){(void)r;(void)m;(void)t;return DS_FAILED;}
int ds_async_queue(struct ds_async_token t,int s,int a){(void)t;(void)s;(void)a;return DS_FAILED;}
int ds_async_start(struct ds_async_token t){(void)t;return DS_FAILED;}
int ds_async_remove(struct ds_async_token t){(void)t;return DS_FAILED;}
int ds_async_drain(struct ds_async_token t,int *out){(void)t;(void)out;return DS_FAILED;}
void ds_reload_init(struct ds_reload *h,struct ds_async_registry *r,struct d_allocator a){(void)h;(void)r;(void)a;}
int ds_reload_change(struct ds_reload *h,const char *p,const int32_t *v,size_t n){(void)h;(void)p;(void)v;(void)n;return DS_FAILED;}
int ds_reload_collect(struct ds_reload *h,ds_collect_gate_fn g,void *c){(void)h;(void)g;(void)c;return DS_FAILED;}
int ds_reload_stop(struct ds_reload *h,ds_collect_gate_fn g,void *c){(void)h;(void)g;(void)c;return DS_FAILED;}
int ds_reload_get(const struct ds_reload *h,size_t i,int32_t *out){(void)h;(void)i;(void)out;return DS_FAILED;}
