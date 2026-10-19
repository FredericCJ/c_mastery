#include "lookup.h"
int sl_normalize(const uint8_t *p,size_t n,uint8_t out[16],size_t *l){(void)p;(void)n;(void)out;(void)l;return STORE_UNIMPLEMENTED;}
int sl_text_bind(store *s,SlText *t){(void)s;(void)t;return STORE_UNIMPLEMENTED;}
int sl_text_put(SlText *s,const void *p,size_t n,const void *v,size_t z){(void)s;(void)p;(void)n;(void)v;(void)z;return STORE_UNIMPLEMENTED;}
int sl_text_get(SlText *s,const void *p,size_t n,void *v,size_t z,size_t *l){(void)s;(void)p;(void)n;(void)v;(void)z;(void)l;return STORE_UNIMPLEMENTED;}
int sl_text_scan(SlText *s,const void *p,size_t n,store_visit f,void *c){(void)s;(void)p;(void)n;(void)f;(void)c;return STORE_UNIMPLEMENTED;}
int sl_snapshot_capture(store *s,SlSnapshot *out){(void)s;(void)out;return STORE_UNIMPLEMENTED;}
int sl_snapshot_scan(const SlSnapshot *s,const void *p,size_t n,size_t b,SlVisit f,void *c,size_t *calls){(void)s;(void)p;(void)n;(void)b;(void)f;(void)c;(void)calls;return STORE_UNIMPLEMENTED;}
