#include "registry.h"
void as_registry_init(struct as_registry *r,unsigned s,as_terminal_fn t,void *c){(void)r;(void)s;(void)t;(void)c;}
enum api_status as_register(struct as_registry *r,as_callback_fn f,void *c,api_drop_fn d,struct api_handle *o){(void)r;(void)f;(void)c;(void)d;(void)o;return API_UNIMPLEMENTED;}
enum api_status as_remove(struct as_registry *r,struct api_handle h){(void)r;(void)h;return API_UNIMPLEMENTED;}
enum api_status as_dispatch(struct as_registry *r){(void)r;return API_UNIMPLEMENTED;}
enum api_status as_destroy(struct as_registry *r){(void)r;return API_UNIMPLEMENTED;}
