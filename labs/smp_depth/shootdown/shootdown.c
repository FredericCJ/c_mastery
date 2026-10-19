#include "shootdown.h"
/* Learner implementation: preserve the declared failure/output contracts. */
int st_init(StWorld *w,unsigned n,uint64_t a,uint64_t b,StPort p){(void)w;(void)n;(void)a;(void)b;(void)p;return ST_INVALID;}
int st_enter(StWorld *w,unsigned c,unsigned s,SlStats *t){(void)w;(void)c;(void)s;(void)t;return ST_INVALID;}
int st_leave(StWorld *w,unsigned c,SlStats *t){(void)w;(void)c;(void)t;return ST_INVALID;}
int st_begin(StWorld *w,unsigned a,unsigned s,uintptr_t f,unsigned n,unsigned k,StRequest *r,SlStats *t){(void)w;(void)a;(void)s;(void)f;(void)n;(void)k;(void)r;(void)t;return ST_INVALID;}
int st_handle(StWorld *w,unsigned c,StRequest r){(void)w;(void)c;(void)r;return ST_INVALID;}
int st_request(const StWorld *w,unsigned s,StRequest *r){(void)w;(void)s;(void)r;return ST_INVALID;}
int st_retry(StWorld *w,StRequest r,SlStats *t){(void)w;(void)r;(void)t;return ST_INVALID;}
int st_finish(StWorld *w,StRequest r,int e,StPermit *p,SlStats *t){(void)w;(void)r;(void)e;(void)p;(void)t;return ST_INVALID;}
