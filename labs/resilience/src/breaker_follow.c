#include "breaker.h"
int rzb_init(struct rzb_gate *g,unsigned a,unsigned b,struct rzb_policy p,uint64_t n){(void)g;(void)a;(void)b;(void)p;(void)n;return RZ_INVALID;}
int rzb_admit(struct rzb_gate *g,unsigned t,unsigned c,struct rzf_scope *s,uint64_t n,struct rzb_permit *p){(void)g;(void)t;(void)c;(void)s;(void)n;(void)p;return RZ_INVALID;}
int rzb_complete(struct rzb_gate *g,struct rzb_permit p,uint64_t n,int o){(void)g;(void)p;(void)n;(void)o;return RZ_INVALID;}
int rzb_cancel(struct rzb_gate *g,struct rzb_permit p){(void)g;(void)p;return RZ_INVALID;}
int rzb_cancelled(struct rzb_gate *g,struct rzb_permit p,uint64_t n,bool *o){(void)g;(void)p;(void)n;(void)o;return RZ_INVALID;}
int rzb_close(struct rzb_gate *g){(void)g;return RZ_INVALID;}
int rzb_quiescent(const struct rzb_gate *g){(void)g;return RZ_INVALID;}
