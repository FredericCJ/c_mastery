#include "ring.h"
/* Intentionally unfinished learner provider. */
enum cr_status cr_build(const struct cr_token *t,size_t n,uint32_t e,struct cr_ring *o){(void)t;(void)n;(void)e;(void)o;return CR_INVALID;}
enum cr_status cr_owner(const struct cr_ring *r,uint16_t k,uint32_t e,unsigned *o,struct cr_cost *c){(void)r;(void)k;(void)e;(void)o;(void)c;return CR_INVALID;}
enum cr_status cr_change(struct cr_ring *r,unsigned o,const struct cr_token *t,size_t n,size_t c){(void)r;(void)o;(void)t;(void)n;(void)c;return CR_INVALID;}
enum cr_status cr_replicas(const struct cr_ring *r,uint16_t k,uint32_t e,unsigned n,uint16_t d,unsigned *o,struct cr_cost *c){(void)r;(void)k;(void)e;(void)n;(void)d;(void)o;(void)c;return CR_INVALID;}
enum cr_status cr_route(const struct cr_ring *r,uint32_t e,const unsigned char *b,size_t n,cr_hash_fn h,void *x,unsigned *o,struct cr_cost *c){(void)r;(void)e;(void)b;(void)n;(void)h;(void)x;(void)o;(void)c;return CR_INVALID;}
enum cr_status cr_plan(const struct cr_ring *a,const struct cr_ring *b,const uint16_t *k,size_t n,struct cr_move *m,size_t z,size_t *c){(void)a;(void)b;(void)k;(void)n;(void)m;(void)z;(void)c;return CR_INVALID;}
