#include "dispatch_deferred.h"
int ds_deferred_init(struct ds_deferred *c){(void)c;return DS_FAILED;}
int ds_deferred_destroy(struct ds_deferred *c){(void)c;return DS_FAILED;}
int ds_deferred_bind(struct ds_deferred *c,struct ow_shared *e,ds_deferred_body_fn b,struct ds_deferred_token *t){(void)c;(void)e;(void)b;(void)t;return DS_FAILED;}
int ds_deferred_submit(struct ds_deferred_token t,int a){(void)t;(void)a;return DS_FAILED;}
int ds_deferred_start(struct ds_deferred_token t){(void)t;return DS_FAILED;}
int ds_deferred_cancel(struct ds_deferred_token t){(void)t;return DS_FAILED;}
int ds_deferred_drain(struct ds_deferred_token t,int *out){(void)t;(void)out;return DS_FAILED;}
