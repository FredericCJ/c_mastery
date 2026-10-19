#include "registry.h"
void as_registry_init(AsRegistry *r,unsigned s,AsTerminal t,void *c){(void)r;(void)s;(void)t;(void)c;}
ApiStatus as_register(AsRegistry *r,AsCallback f,void *c,ApiDrop d,ApiHandle *o){(void)r;(void)f;(void)c;(void)d;(void)o;return API_UNIMPLEMENTED;}
ApiStatus as_remove(AsRegistry *r,ApiHandle h){(void)r;(void)h;return API_UNIMPLEMENTED;}
ApiStatus as_dispatch(AsRegistry *r){(void)r;return API_UNIMPLEMENTED;}
ApiStatus as_destroy(AsRegistry *r){(void)r;return API_UNIMPLEMENTED;}
