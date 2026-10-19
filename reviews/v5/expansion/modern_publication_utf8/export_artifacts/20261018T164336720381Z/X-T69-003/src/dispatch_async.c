#include "dispatch_async.h"
int ds_async_init(DSAsyncRegistry *r){(void)r;return DS_FAILED;}
int ds_async_destroy(DSAsyncRegistry *r){(void)r;return DS_FAILED;}
int ds_async_register(DSAsyncRegistry *r,DSLoaded *m,DSAsyncToken *t){(void)r;(void)m;(void)t;return DS_FAILED;}
int ds_async_queue(DSAsyncToken t,int s,int a){(void)t;(void)s;(void)a;return DS_FAILED;}
int ds_async_start(DSAsyncToken t){(void)t;return DS_FAILED;}
int ds_async_remove(DSAsyncToken t){(void)t;return DS_FAILED;}
int ds_async_drain(DSAsyncToken t,int *out){(void)t;(void)out;return DS_FAILED;}
void ds_reload_init(DSReload *h,DSAsyncRegistry *r,DAllocator a){(void)h;(void)r;(void)a;}
int ds_reload_change(DSReload *h,const char *p,const int32_t *v,size_t n){(void)h;(void)p;(void)v;(void)n;return DS_FAILED;}
int ds_reload_collect(DSReload *h,DSCollectGate g,void *c){(void)h;(void)g;(void)c;return DS_FAILED;}
int ds_reload_stop(DSReload *h,DSCollectGate g,void *c){(void)h;(void)g;(void)c;return DS_FAILED;}
int ds_reload_get(const DSReload *h,size_t i,int32_t *out){(void)h;(void)i;(void)out;return DS_FAILED;}
