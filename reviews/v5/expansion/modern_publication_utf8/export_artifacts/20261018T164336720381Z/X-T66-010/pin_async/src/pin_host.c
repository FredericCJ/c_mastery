#include "pin_host.h"
OwStatus opa_port_init(OpaPort *p,int32_t *b,size_t n){(void)p;(void)b;(void)n;return OW_NOT_IMPLEMENTED;}
OwStatus opa_port_start(OpaPort *p,int32_t v,OpaReceiver f,void *c,uint32_t *t){(void)p;(void)v;(void)f;(void)c;(void)t;return OW_NOT_IMPLEMENTED;}
OwStatus opa_port_cancel(OpaPort *p,uint32_t t,int e){(void)p;(void)t;(void)e;return OW_NOT_IMPLEMENTED;}
OwStatus opa_port_allow_backend(OpaPort *p,uint32_t t){(void)p;(void)t;return OW_NOT_IMPLEMENTED;}
OwStatus opa_port_observe(OpaPort *p,OpaObservation *o){(void)p;(void)o;return OW_NOT_IMPLEMENTED;}
OwStatus opa_port_join(OpaPort *p,uint32_t t,int w,int32_t *o){(void)p;(void)t;(void)w;(void)o;return OW_NOT_IMPLEMENTED;}
OwStatus opa_port_rebind(OpaPort *p,int32_t *b,size_t n){(void)p;(void)b;(void)n;return OW_NOT_IMPLEMENTED;}
OwStatus opa_port_destroy(OpaPort *p){(void)p;return OW_NOT_IMPLEMENTED;}
