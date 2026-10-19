#include "erase.h"
/* Compiling incomplete learner component. */
int er_init(ErOwner*a,const ErItem*b,size_t n){(void)b;(void)n;if(a)*a=(ErOwner){0};return ER_INVALID;}
int er_compact(int*a,size_t n,int v,size_t*out,size_t*m){(void)a;(void)n;(void)v;(void)out;(void)m;return ER_INVALID;}
int er_remove_if(ErOwner*a,ErPredicate p,void*c,ErTail*out){(void)a;(void)p;(void)c;(void)out;return ER_INVALID;}
int er_erase(ErOwner*a,ErTail t,ErDrop d,void*c){(void)a;(void)t;(void)d;(void)c;return ER_INVALID;}
int er_clear(ErOwner*a,ErDrop d,void*c){(void)a;(void)d;(void)c;return ER_INVALID;}
