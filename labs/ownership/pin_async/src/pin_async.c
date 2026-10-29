#include "pin_async.h"
enum ow_status opa_init(struct opa_machine *m,const void *a,uint64_t o,uint64_t e){(void)m;(void)a;(void)o;(void)e;return OW_NOT_IMPLEMENTED;}
enum ow_status opa_submit(struct opa_machine *m,const void *a,uint64_t o,uint32_t *t){(void)m;(void)a;(void)o;(void)t;return OW_NOT_IMPLEMENTED;}
enum ow_status opa_cancel(struct opa_machine *m,uint32_t t,uint64_t o){(void)m;(void)t;(void)o;return OW_NOT_IMPLEMENTED;}
enum ow_status opa_timeout(struct opa_machine *m,uint32_t t,uint64_t o){(void)m;(void)t;(void)o;return OW_NOT_IMPLEMENTED;}
enum ow_status opa_notify(struct opa_machine *m,uint32_t t,uint64_t e,int32_t v){(void)m;(void)t;(void)e;(void)v;return OW_NOT_IMPLEMENTED;}
enum ow_status opa_finish(struct opa_machine *m,uint32_t t,uint64_t o,int32_t *v){(void)m;(void)t;(void)o;(void)v;return OW_NOT_IMPLEMENTED;}
enum ow_status opa_rebind(struct opa_machine *m,const void *a,uint64_t o){(void)m;(void)a;(void)o;return OW_NOT_IMPLEMENTED;}
