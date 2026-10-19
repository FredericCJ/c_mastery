#include "ring.h"
/* Intentionally unfinished learner provider. */
CrStatus cr_build(const CrToken *t,size_t n,uint32_t e,CrRing *o){(void)t;(void)n;(void)e;(void)o;return CR_INVALID;}
CrStatus cr_owner(const CrRing *r,uint16_t k,uint32_t e,unsigned *o,CrCost *c){(void)r;(void)k;(void)e;(void)o;(void)c;return CR_INVALID;}
CrStatus cr_change(CrRing *r,unsigned o,const CrToken *t,size_t n,size_t c){(void)r;(void)o;(void)t;(void)n;(void)c;return CR_INVALID;}
CrStatus cr_replicas(const CrRing *r,uint16_t k,uint32_t e,unsigned n,uint16_t d,unsigned *o,CrCost *c){(void)r;(void)k;(void)e;(void)n;(void)d;(void)o;(void)c;return CR_INVALID;}
CrStatus cr_route(const CrRing *r,uint32_t e,const unsigned char *b,size_t n,CrHash h,void *x,unsigned *o,CrCost *c){(void)r;(void)e;(void)b;(void)n;(void)h;(void)x;(void)o;(void)c;return CR_INVALID;}
CrStatus cr_plan(const CrRing *a,const CrRing *b,const uint16_t *k,size_t n,CrMove *m,size_t z,size_t *c){(void)a;(void)b;(void)k;(void)n;(void)m;(void)z;(void)c;return CR_INVALID;}
