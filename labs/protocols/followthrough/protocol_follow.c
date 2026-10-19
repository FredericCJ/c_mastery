#include "protocol_follow.h"
int pf_driver_init(PFDriver *d,FwIo io,uint32_t e,PFFree f,void *c)
{(void)d;(void)io;(void)e;(void)f;(void)c;return PT_UNIMPLEMENTED;}
int pf_driver_ready(PFDriver *d){(void)d;return PT_UNIMPLEMENTED;}
int pf_driver_start(PFDriver *d,unsigned char **p,size_t n,unsigned f,PFRecovery *r)
{(void)d;(void)p;(void)n;(void)f;(void)r;return PT_UNIMPLEMENTED;}
int pf_driver_reset(PFRecovery *r){(void)r;return PT_UNIMPLEMENTED;}
int pf_driver_recover(PFRecovery *r,uint32_t e,unsigned i){(void)r;(void)e;(void)i;return PT_UNIMPLEMENTED;}
int pf_driver_complete(PFRecovery *r,uint32_t e,unsigned i){(void)r;(void)e;(void)i;return PT_UNIMPLEMENTED;}
int pf_builder_mode(PFBuilder *b,unsigned m,unsigned c){(void)b;(void)m;(void)c;return PT_UNIMPLEMENTED;}
int pf_builder_replace(PFBuilder *b,unsigned f,unsigned v){(void)b;(void)f;(void)v;return PT_UNIMPLEMENTED;}
int pf_builder_finish(const PFBuilder *b,PFConfig *o){(void)b;(void)o;return PT_UNIMPLEMENTED;}
int pf_session(PFSession *s,unsigned e,uint32_t n,int32_t v){(void)s;(void)e;(void)n;(void)v;return PT_UNIMPLEMENTED;}
int pf_decoder_init(PFDecoder *d,uint64_t t,PFAlloc a,PFFree f,void *c){(void)d;(void)t;(void)a;(void)f;(void)c;return PT_UNIMPLEMENTED;}
int pf_decoder_step(PFDecoder *d,unsigned a,unsigned char v,uint64_t t,unsigned char **o,size_t *n){(void)d;(void)a;(void)v;(void)t;(void)o;(void)n;return PT_UNIMPLEMENTED;}
int pf_decoder_cancel(PFDecoder *d){(void)d;return PT_UNIMPLEMENTED;}
