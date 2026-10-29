#include "strides.h"
enum ow_status osv_indices(size_t n,struct osv_shape s,size_t o[64],size_t *c){(void)n;(void)s;(void)o;(void)c;return OW_NOT_IMPLEMENTED;}
enum ow_status osv_acquire(struct osv_book *b,struct osv_shape s,int w,struct osv_token *t){(void)b;(void)s;(void)w;(void)t;return OW_NOT_IMPLEMENTED;}
enum ow_status osv_release(struct osv_book *b,struct osv_token t){(void)b;(void)t;return OW_NOT_IMPLEMENTED;}
enum ow_status osv_split(struct osv_book *b,struct osv_token t,unsigned a,size_t c,struct osv_token o[2]){(void)b;(void)t;(void)a;(void)c;(void)o;return OW_NOT_IMPLEMENTED;}
enum ow_status osv_read(const struct osv_book *b,struct osv_token t,const int32_t *d,size_t n,size_t r,size_t c,int32_t *o){(void)b;(void)t;(void)d;(void)n;(void)r;(void)c;(void)o;return OW_NOT_IMPLEMENTED;}
enum ow_status osv_write(struct osv_book *b,struct osv_token t,int32_t *d,size_t n,size_t r,size_t c,int32_t v){(void)b;(void)t;(void)d;(void)n;(void)r;(void)c;(void)v;return OW_NOT_IMPLEMENTED;}
