#include "protocol_follow.h"
int pf_driver_init(struct pf_driver *d,struct fw_io io,uint32_t e,pf_free_fn f,void *c)
{(void)d;(void)io;(void)e;(void)f;(void)c;return PT_UNIMPLEMENTED;}
int pf_driver_ready(struct pf_driver *d){(void)d;return PT_UNIMPLEMENTED;}
int pf_driver_start(struct pf_driver *d,unsigned char **p,size_t n,unsigned f,struct pf_recovery *r)
{(void)d;(void)p;(void)n;(void)f;(void)r;return PT_UNIMPLEMENTED;}
int pf_driver_reset(struct pf_recovery *r){(void)r;return PT_UNIMPLEMENTED;}
int pf_driver_recover(struct pf_recovery *r,uint32_t e,unsigned i){(void)r;(void)e;(void)i;return PT_UNIMPLEMENTED;}
int pf_driver_complete(struct pf_recovery *r,uint32_t e,unsigned i){(void)r;(void)e;(void)i;return PT_UNIMPLEMENTED;}
int pf_builder_mode(struct pf_builder *b,unsigned m,unsigned c){(void)b;(void)m;(void)c;return PT_UNIMPLEMENTED;}
int pf_builder_replace(struct pf_builder *b,unsigned f,unsigned v){(void)b;(void)f;(void)v;return PT_UNIMPLEMENTED;}
int pf_builder_finish(const struct pf_builder *b,struct pf_config *o){(void)b;(void)o;return PT_UNIMPLEMENTED;}
int pf_session(struct pf_session *s,unsigned e,uint32_t n,int32_t v){(void)s;(void)e;(void)n;(void)v;return PT_UNIMPLEMENTED;}
int pf_decoder_init(struct pf_decoder *d,uint64_t t,pf_alloc_fn a,pf_free_fn f,void *c){(void)d;(void)t;(void)a;(void)f;(void)c;return PT_UNIMPLEMENTED;}
int pf_decoder_step(struct pf_decoder *d,unsigned a,unsigned char v,uint64_t t,unsigned char **o,size_t *n){(void)d;(void)a;(void)v;(void)t;(void)o;(void)n;return PT_UNIMPLEMENTED;}
int pf_decoder_cancel(struct pf_decoder *d){(void)d;return PT_UNIMPLEMENTED;}
