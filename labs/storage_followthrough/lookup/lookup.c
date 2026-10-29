#include "lookup.h"
int sl_normalize(const uint8_t *p,size_t n,uint8_t out[16],size_t *l){(void)p;(void)n;(void)out;(void)l;return STORE_UNIMPLEMENTED;}
int sl_text_bind(store *s,struct sl_text *t){(void)s;(void)t;return STORE_UNIMPLEMENTED;}
int sl_text_put(struct sl_text *s,const void *p,size_t n,const void *v,size_t z){(void)s;(void)p;(void)n;(void)v;(void)z;return STORE_UNIMPLEMENTED;}
int sl_text_get(struct sl_text *s,const void *p,size_t n,void *v,size_t z,size_t *l){(void)s;(void)p;(void)n;(void)v;(void)z;(void)l;return STORE_UNIMPLEMENTED;}
int sl_text_scan(struct sl_text *s,const void *p,size_t n,store_visit f,void *c){(void)s;(void)p;(void)n;(void)f;(void)c;return STORE_UNIMPLEMENTED;}
int sl_snapshot_capture(store *s,struct sl_snapshot *out){(void)s;(void)out;return STORE_UNIMPLEMENTED;}
int sl_snapshot_scan(const struct sl_snapshot *s,const void *p,size_t n,size_t b,sl_visit_fn f,void *c,size_t *calls){(void)s;(void)p;(void)n;(void)b;(void)f;(void)c;(void)calls;return STORE_UNIMPLEMENTED;}
