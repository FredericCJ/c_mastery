#include "progress.h"
void ept_init(struct ept_stream *s){(void)s;}
unsigned ept_start(struct ept_stream *s,const unsigned char *b,size_t n,struct ept_handle *h){(void)s;(void)b;(void)n;(void)h;return EPT_UNIMPLEMENTED;}
unsigned ept_step(struct ept_handle h,er_sink f,void *c,struct ept_observation *o){(void)h;(void)f;(void)c;(void)o;return EPT_UNIMPLEMENTED;}
unsigned ept_cancel(struct ept_handle h,struct ept_observation *o){(void)h;(void)o;return EPT_UNIMPLEMENTED;}
unsigned ept_checkpoint(struct ept_handle h,ept_flush_fn f,void *c,struct ept_observation *o){(void)h;(void)f;(void)c;(void)o;return EPT_UNIMPLEMENTED;}
unsigned ept_reconcile(struct ept_handle h,size_t n,uint32_t e,unsigned q,struct ept_observation *o){(void)h;(void)n;(void)e;(void)q;(void)o;return EPT_UNIMPLEMENTED;}
unsigned ept_observe(struct ept_handle h,struct ept_observation *o){(void)h;(void)o;return EPT_UNIMPLEMENTED;}
unsigned ept_release(struct ept_handle h,unsigned q,unsigned a){(void)h;(void)q;(void)a;return EPT_UNIMPLEMENTED;}
