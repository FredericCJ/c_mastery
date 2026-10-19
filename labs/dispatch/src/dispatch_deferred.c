#include "dispatch_deferred.h"
int ds_deferred_init(DSDeferred *c){(void)c;return DS_FAILED;}
int ds_deferred_destroy(DSDeferred *c){(void)c;return DS_FAILED;}
int ds_deferred_bind(DSDeferred *c,OwShared *e,DSDeferredBody b,DSDeferredToken *t){(void)c;(void)e;(void)b;(void)t;return DS_FAILED;}
int ds_deferred_submit(DSDeferredToken t,int a){(void)t;(void)a;return DS_FAILED;}
int ds_deferred_start(DSDeferredToken t){(void)t;return DS_FAILED;}
int ds_deferred_cancel(DSDeferredToken t){(void)t;return DS_FAILED;}
int ds_deferred_drain(DSDeferredToken t,int *out){(void)t;(void)out;return DS_FAILED;}
