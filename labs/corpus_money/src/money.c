#include "money.h"
/* Learner scaffold: complete the contract, not this refusal's behavior. */
enum cm_status cm_make(int64_t a,const char c[3],unsigned s,struct cm_money *o){(void)a;(void)c;(void)s;(void)o;return CM_INVALID;}
enum cm_status cm_add(struct cm_money a,struct cm_money b,struct cm_money *o){(void)a;(void)b;(void)o;return CM_INVALID;}
enum cm_status cm_ratio(struct cm_money a,uint32_t n,uint32_t d,enum cm_round r,struct cm_money *o){(void)a;(void)n;(void)d;(void)r;(void)o;return CM_INVALID;}
enum cm_status cm_allocate(struct cm_money a,const uint32_t *w,size_t n,struct cm_money *o){(void)a;(void)w;(void)n;(void)o;return CM_INVALID;}
enum cm_status cm_settle(struct cm_money *a,size_t n,size_t f,const size_t *d,const uint32_t *w,size_t k,struct cm_money m,size_t c){(void)a;(void)n;(void)f;(void)d;(void)w;(void)k;(void)m;(void)c;return CM_INVALID;}
