#include "higher_order.h"
int ho_pure(struct ho_hoa *a,int32_t v,unsigned *out){(void)a;(void)v;(void)out;return HO_FAILED;}
int ho_emit(struct ho_hoa *a,int32_t v,unsigned n,unsigned *out){(void)a;(void)v;(void)n;(void)out;return HO_FAILED;}
int ho_choose(struct ho_hoa *a,unsigned l,unsigned r,unsigned *out){(void)a;(void)l;(void)r;(void)out;return HO_FAILED;}
int ho_bind(struct ho_hoa *a,unsigned n,ho_bind_fn f,void *c,unsigned *out){(void)a;(void)n;(void)f;(void)c;(void)out;return HO_FAILED;}
int ho_run(const struct ho_hoa *a,unsigned n,const unsigned char *p,size_t z,struct ho_result *out){(void)a;(void)n;(void)p;(void)z;(void)out;return HO_FAILED;}
int ho_summarize(const struct ho_hoa *a,unsigned n,struct ho_summary *out){(void)a;(void)n;(void)out;return HO_FAILED;}
