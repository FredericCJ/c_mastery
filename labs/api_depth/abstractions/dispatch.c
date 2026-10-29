#include "dispatch.h"
/* Learner-owned transport dispatch mechanism; provider backends supplied separately. */
int ad_direct_init(struct ad_direct *d,unsigned k){(void)d;(void)k;return SF_UNIMPLEMENTED;}
int ad_erased_init(struct ad_erased *e,const struct ad_ops *o){(void)e;(void)o;return SF_UNIMPLEMENTED;}
ptrdiff_t ad_direct_write(void *c,const unsigned char *p,size_t n){(void)c;(void)p;(void)n;return -1;}
ptrdiff_t ad_erased_write(void *c,const unsigned char *p,size_t n){(void)c;(void)p;(void)n;return -1;}
ptrdiff_t ad_fixed_write(void *c,const unsigned char *p,size_t n){(void)c;(void)p;(void)n;return -1;}
