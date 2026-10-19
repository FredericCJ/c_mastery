#include "pin_async.h"
OwStatus opa_init(OpaMachine *m,const void *a,uint64_t o,uint64_t e){(void)m;(void)a;(void)o;(void)e;return OW_NOT_IMPLEMENTED;}
OwStatus opa_submit(OpaMachine *m,const void *a,uint64_t o,uint32_t *t){(void)m;(void)a;(void)o;(void)t;return OW_NOT_IMPLEMENTED;}
OwStatus opa_cancel(OpaMachine *m,uint32_t t,uint64_t o){(void)m;(void)t;(void)o;return OW_NOT_IMPLEMENTED;}
OwStatus opa_timeout(OpaMachine *m,uint32_t t,uint64_t o){(void)m;(void)t;(void)o;return OW_NOT_IMPLEMENTED;}
OwStatus opa_notify(OpaMachine *m,uint32_t t,uint64_t e,int32_t v){(void)m;(void)t;(void)e;(void)v;return OW_NOT_IMPLEMENTED;}
OwStatus opa_finish(OpaMachine *m,uint32_t t,uint64_t o,int32_t *v){(void)m;(void)t;(void)o;(void)v;return OW_NOT_IMPLEMENTED;}
OwStatus opa_rebind(OpaMachine *m,const void *a,uint64_t o){(void)m;(void)a;(void)o;return OW_NOT_IMPLEMENTED;}
