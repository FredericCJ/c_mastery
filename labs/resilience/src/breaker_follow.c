#include "breaker.h"
int rzb_init(RzbGate *g,unsigned a,unsigned b,RzbPolicy p,uint64_t n){(void)g;(void)a;(void)b;(void)p;(void)n;return RZ_INVALID;}
int rzb_admit(RzbGate *g,unsigned t,unsigned c,RzfScope *s,uint64_t n,RzbPermit *p){(void)g;(void)t;(void)c;(void)s;(void)n;(void)p;return RZ_INVALID;}
int rzb_complete(RzbGate *g,RzbPermit p,uint64_t n,int o){(void)g;(void)p;(void)n;(void)o;return RZ_INVALID;}
int rzb_cancel(RzbGate *g,RzbPermit p){(void)g;(void)p;return RZ_INVALID;}
int rzb_cancelled(RzbGate *g,RzbPermit p,uint64_t n,bool *o){(void)g;(void)p;(void)n;(void)o;return RZ_INVALID;}
int rzb_close(RzbGate *g){(void)g;return RZ_INVALID;}
int rzb_quiescent(const RzbGate *g){(void)g;return RZ_INVALID;}
