#include "higher_order.h"
int ho_pure(HOA *a,int32_t v,unsigned *out){(void)a;(void)v;(void)out;return HO_FAILED;}
int ho_emit(HOA *a,int32_t v,unsigned n,unsigned *out){(void)a;(void)v;(void)n;(void)out;return HO_FAILED;}
int ho_choose(HOA *a,unsigned l,unsigned r,unsigned *out){(void)a;(void)l;(void)r;(void)out;return HO_FAILED;}
int ho_bind(HOA *a,unsigned n,HOBind f,void *c,unsigned *out){(void)a;(void)n;(void)f;(void)c;(void)out;return HO_FAILED;}
int ho_run(const HOA *a,unsigned n,const unsigned char *p,size_t z,HOResult *out){(void)a;(void)n;(void)p;(void)z;(void)out;return HO_FAILED;}
int ho_summarize(const HOA *a,unsigned n,HOSummary *out){(void)a;(void)n;(void)out;return HO_FAILED;}
