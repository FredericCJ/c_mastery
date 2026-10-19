#include "money.h"
/* Learner scaffold: complete the contract, not this refusal's behavior. */
CmStatus cm_make(int64_t a,const char c[3],unsigned s,CmMoney *o){(void)a;(void)c;(void)s;(void)o;return CM_INVALID;}
CmStatus cm_add(CmMoney a,CmMoney b,CmMoney *o){(void)a;(void)b;(void)o;return CM_INVALID;}
CmStatus cm_ratio(CmMoney a,uint32_t n,uint32_t d,CmRound r,CmMoney *o){(void)a;(void)n;(void)d;(void)r;(void)o;return CM_INVALID;}
CmStatus cm_allocate(CmMoney a,const uint32_t *w,size_t n,CmMoney *o){(void)a;(void)w;(void)n;(void)o;return CM_INVALID;}
CmStatus cm_settle(CmMoney *a,size_t n,size_t f,const size_t *d,const uint32_t *w,size_t k,CmMoney m,size_t c){(void)a;(void)n;(void)f;(void)d;(void)w;(void)k;(void)m;(void)c;return CM_INVALID;}
