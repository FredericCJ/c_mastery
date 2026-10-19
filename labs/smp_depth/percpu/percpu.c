#include "percpu.h"
int sp_layout(uint64_t b,uint64_t n,unsigned c,uint64_t s,unsigned a,SpLayout *o){(void)b;(void)n;(void)c;(void)s;(void)a;(void)o;return SP_INVALID;}
int sp_validate(const SpSpan *s,size_t n){(void)s;(void)n;return SP_INVALID;}
int sp_budget(const uint64_t *f,size_t n,uint64_t h,uint64_t s,unsigned a,uint64_t e,uint64_t *o){(void)f;(void)n;(void)h;(void)s;(void)a;(void)e;(void)o;return SP_INVALID;}
