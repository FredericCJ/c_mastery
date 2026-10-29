#include "ownership.h"
/* Unsolved learner core; target application is a separate learner artifact. */
#define UNUSED(x) (void)(x)
int cr_pool_init(struct cr_pool *p,uint64_t w){UNUSED(p);UNUSED(w);return CR_INVALID;}
int cr_allocate(struct cr_pool *p,int f,const unsigned char *b,size_t n,struct cr_token *t){UNUSED(p);UNUSED(f);UNUSED(b);UNUSED(n);UNUSED(t);return CR_INVALID;}
int cr_offer(struct cr_pool *p,struct cr_token t){UNUSED(p);UNUSED(t);return CR_INVALID;}
int cr_abort_offer(struct cr_pool *p,struct cr_token t){UNUSED(p);UNUSED(t);return CR_INVALID;}
int cr_claim(struct cr_pool *p,struct cr_token t){UNUSED(p);UNUSED(t);return CR_INVALID;}
int cr_read(const struct cr_pool *p,struct cr_token t,unsigned char *b,size_t n,size_t *o){UNUSED(p);UNUSED(t);UNUSED(b);UNUSED(n);UNUSED(o);return CR_INVALID;}
int cr_release(struct cr_pool *p,struct cr_token t){UNUSED(p);UNUSED(t);return CR_INVALID;}
int cr_close(struct cr_pool *p){UNUSED(p);return CR_INVALID;}
int cr_drained(const struct cr_pool *p){UNUSED(p);return CR_INVALID;}
int cr_budget(const uint64_t *p,size_t n,int k,uint64_t a,uint64_t *o){UNUSED(p);UNUSED(n);UNUSED(k);UNUSED(a);UNUSED(o);return CR_INVALID;}
