/* Intentionally unsolved learner scaffold. */
#include "checkpoint_numeric.h"
enum cpn_status cpn_transaction(struct cpn_double *s,const double *i,size_t n,double *o,size_t c,size_t f,int a,size_t *k){(void)s;(void)i;(void)n;(void)o;(void)c;(void)f;(void)a;(void)k;return CPN_INVALID;}
enum cpn_status cpn_round_even(int64_t n,int32_t *o){(void)n;(void)o;return CPN_INVALID;}
enum cpn_status cpn_iir_half(double *s,const double *i,size_t n,double *o,size_t c){(void)s;(void)i;(void)n;(void)o;(void)c;return CPN_INVALID;}
void cpn_exact_init(struct cpn_exact*s){s->previous=0;s->older=0;}
void cpn_double_init(struct cpn_double*s){s->previous=0;s->older=0;}
enum cpn_status cpn_exact(struct cpn_exact*s,const int32_t*i,size_t n,int64_t*o,size_t c){(void)s;(void)i;(void)n;(void)o;(void)c;return CPN_INVALID;}
enum cpn_status cpn_double(struct cpn_double*s,const double*i,size_t n,double*o,size_t c){(void)s;(void)i;(void)n;(void)o;(void)c;return CPN_INVALID;}
