#include "strides.h"
OwStatus osv_indices(size_t n,OsvShape s,size_t o[64],size_t *c){(void)n;(void)s;(void)o;(void)c;return OW_NOT_IMPLEMENTED;}
OwStatus osv_acquire(OsvBook *b,OsvShape s,int w,OsvToken *t){(void)b;(void)s;(void)w;(void)t;return OW_NOT_IMPLEMENTED;}
OwStatus osv_release(OsvBook *b,OsvToken t){(void)b;(void)t;return OW_NOT_IMPLEMENTED;}
OwStatus osv_split(OsvBook *b,OsvToken t,unsigned a,size_t c,OsvToken o[2]){(void)b;(void)t;(void)a;(void)c;(void)o;return OW_NOT_IMPLEMENTED;}
OwStatus osv_read(const OsvBook *b,OsvToken t,const int32_t *d,size_t n,size_t r,size_t c,int32_t *o){(void)b;(void)t;(void)d;(void)n;(void)r;(void)c;(void)o;return OW_NOT_IMPLEMENTED;}
OwStatus osv_write(OsvBook *b,OsvToken t,int32_t *d,size_t n,size_t r,size_t c,int32_t v){(void)b;(void)t;(void)d;(void)n;(void)r;(void)c;(void)v;return OW_NOT_IMPLEMENTED;}
