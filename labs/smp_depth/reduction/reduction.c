#include "reduction.h"
int sr_partition(size_t n,unsigned p,unsigned i,SrRange *out){(void)n;(void)p;(void)i;(void)out;return SR_INVALID;}
int sr_init(SrWorld *w,const void *v,atomic_uint *a,size_t n,unsigned p,unsigned m){(void)w;(void)v;(void)a;(void)n;(void)p;(void)m;return SR_INVALID;}
int sr_work(SrWorld *w,unsigned i,size_t f){(void)w;(void)i;(void)f;return SR_INVALID;}
int sr_finish(const SrWorld *w,SrResult *r){(void)w;(void)r;return SR_INVALID;}
