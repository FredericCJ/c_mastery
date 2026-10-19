#include "event_follow.h"
int pf_post(PFHsm *h,unsigned e){(void)h;(void)e;return PT_UNIMPLEMENTED;}
int pf_dispatch(PFHsm *h,unsigned e,unsigned g,unsigned l,PFEntry f,void *c,unsigned char *t,size_t n,size_t *u){(void)h;(void)e;(void)g;(void)l;(void)f;(void)c;(void)t;(void)n;(void)u;return PT_UNIMPLEMENTED;}
int pf_active_post(PFActive *a,unsigned e){(void)a;(void)e;return PT_UNIMPLEMENTED;}
int pf_active_step(PFActive *a,unsigned char *t,size_t n,size_t *u){(void)a;(void)t;(void)n;(void)u;return PT_UNIMPLEMENTED;}
int pf_active_discard(PFActive *a,unsigned *e){(void)a;(void)e;return PT_UNIMPLEMENTED;}
int pf_reactive_init(PFReactive *b,const PFNode *p,size_t n){(void)b;(void)p;(void)n;return PT_UNIMPLEMENTED;}
int pf_reactive_tick(PFReactive *b,const PFNode *p,size_t n,unsigned g,PFLeaf l,PFAbort a,void *c,unsigned *o){(void)b;(void)p;(void)n;(void)g;(void)l;(void)a;(void)c;(void)o;return PT_UNIMPLEMENTED;}
int pf_reactive_cancel(PFReactive *b,PFAbort a,void *c){(void)b;(void)a;(void)c;return PT_UNIMPLEMENTED;}
