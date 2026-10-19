/* Intentionally unsolved learner scaffold. */
#include "checkpoint_numeric.h"
CPNStatus cpn_transaction(CPNDouble *s,const double *i,size_t n,double *o,size_t c,size_t f,int a,size_t *k){(void)s;(void)i;(void)n;(void)o;(void)c;(void)f;(void)a;(void)k;return CPN_INVALID;}
CPNStatus cpn_round_even(int64_t n,int32_t *o){(void)n;(void)o;return CPN_INVALID;}
CPNStatus cpn_iir_half(double *s,const double *i,size_t n,double *o,size_t c){(void)s;(void)i;(void)n;(void)o;(void)c;return CPN_INVALID;}
void cpn_exact_init(CPNExact*s){s->previous=0;s->older=0;}
void cpn_double_init(CPNDouble*s){s->previous=0;s->older=0;}
CPNStatus cpn_exact(CPNExact*s,const int32_t*i,size_t n,int64_t*o,size_t c){(void)s;(void)i;(void)n;(void)o;(void)c;return CPN_INVALID;}
CPNStatus cpn_double(CPNDouble*s,const double*i,size_t n,double*o,size_t c){(void)s;(void)i;(void)n;(void)o;(void)c;return CPN_INVALID;}
