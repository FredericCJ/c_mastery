#include "ownership.h"
/* Unsolved learner core; target application is a separate learner artifact. */
#define UNUSED(x) (void)(x)
int cr_pool_init(CrPool *p,uint64_t w){UNUSED(p);UNUSED(w);return CR_INVALID;}
int cr_allocate(CrPool *p,int f,const unsigned char *b,size_t n,CrToken *t){UNUSED(p);UNUSED(f);UNUSED(b);UNUSED(n);UNUSED(t);return CR_INVALID;}
int cr_offer(CrPool *p,CrToken t){UNUSED(p);UNUSED(t);return CR_INVALID;}
int cr_abort_offer(CrPool *p,CrToken t){UNUSED(p);UNUSED(t);return CR_INVALID;}
int cr_claim(CrPool *p,CrToken t){UNUSED(p);UNUSED(t);return CR_INVALID;}
int cr_read(const CrPool *p,CrToken t,unsigned char *b,size_t n,size_t *o){UNUSED(p);UNUSED(t);UNUSED(b);UNUSED(n);UNUSED(o);return CR_INVALID;}
int cr_release(CrPool *p,CrToken t){UNUSED(p);UNUSED(t);return CR_INVALID;}
int cr_close(CrPool *p){UNUSED(p);return CR_INVALID;}
int cr_drained(const CrPool *p){UNUSED(p);return CR_INVALID;}
int cr_budget(const uint64_t *p,size_t n,int k,uint64_t a,uint64_t *o){UNUSED(p);UNUSED(n);UNUSED(k);UNUSED(a);UNUSED(o);return CR_INVALID;}
