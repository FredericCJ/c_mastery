#include "relocating_pool.h"
AdpStatus adp_ids_init(AdpIds *s,uint64_t n){(void)s;(void)n;return ADP_INVALID;}
AdpStatus adp_issue(AdpIds *s,AdpId *o){(void)s;(void)o;return ADP_INVALID;}
AdpStatus adp_init(AdpPool *p,AdpId i,AdpOps o,uint64_t g){(void)p;(void)i;(void)o;(void)g;return ADP_INVALID;}
AdpStatus adp_acquire(AdpPool *p,uint32_t v,AdpHandle *o){(void)p;(void)v;(void)o;return ADP_INVALID;}
AdpStatus adp_get(AdpPool *p,AdpHandle h,uint32_t *o){(void)p;(void)h;(void)o;return ADP_INVALID;}
AdpStatus adp_release(AdpPool *p,AdpHandle h){(void)p;(void)h;return ADP_INVALID;}
AdpStatus adp_snapshot(AdpPool *p,AdpSnapshot *o){(void)p;(void)o;return ADP_INVALID;}
AdpStatus adp_compact(AdpPool *p){(void)p;return ADP_INVALID;}
AdpStatus adp_move(AdpPool *d,AdpPool *s){(void)d;(void)s;return ADP_INVALID;}
AdpStatus adp_close(AdpPool *p){(void)p;return ADP_INVALID;}
AdpStatus adp_destroy(AdpPool *p){(void)p;return ADP_INVALID;}
