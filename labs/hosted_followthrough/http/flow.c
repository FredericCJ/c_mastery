#include "flow.h"
/* Intentionally incomplete learner implementation; no assessor fallback. */
int hf_admit(HFPool *p,int d,unsigned f,HFHandle *h){(void)p;(void)d;(void)f;(void)h;return -1;}
int hf_pin(HFPool *p,HFHandle h){(void)p;(void)h;return -1;}
int hf_callback(HFPool *p,HFHandle h,int *d){(void)p;(void)h;(void)d;return -1;}
int hf_close(HFPool *p,HFHandle h,int *d){(void)p;(void)h;(void)d;return -1;}
int hf_accept_batch(HFPool *p,HFAccept o,HFHandle h[4],unsigned *c){(void)p;(void)o;(void)h;(void)c;return -1;}
int hf_response(const HttpRequest *r,const char *d,int c,const void *e,size_t z,void *o,size_t n,size_t *l){(void)r;(void)d;(void)c;(void)e;(void)z;(void)o;(void)n;(void)l;return -1;}
int hf_init(HFFlow *f,unsigned c,const char *a,uint64_t n){(void)f;(void)c;(void)a;(void)n;return -1;}
int hf_feed(HFFlow *f,const void *p,size_t n,size_t *u,uint64_t t,const char *a,const char *d){(void)f;(void)p;(void)n;(void)u;(void)t;(void)a;(void)d;return -1;}
int hf_eof(HFFlow *f,uint64_t n,const char *d){(void)f;(void)n;(void)d;return -1;}
int hf_read_event(HFFlow *f,int r,uint64_t n,const char *d){(void)f;(void)r;(void)n;(void)d;return -1;}
int hf_write(HFFlow *f,HttpWrite w,void *c,uint64_t n){(void)f;(void)w;(void)c;(void)n;return -1;}
int hf_tick(HFFlow *f,uint64_t n,const char *d){(void)f;(void)n;(void)d;return -1;}
