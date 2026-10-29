#include "shootdown.h"
/* Learner implementation: preserve the declared failure/output contracts. */
int st_init(struct st_world *w,unsigned n,uint64_t a,uint64_t b,struct st_port p){(void)w;(void)n;(void)a;(void)b;(void)p;return ST_INVALID;}
int st_enter(struct st_world *w,unsigned c,unsigned s,struct sl_stats *t){(void)w;(void)c;(void)s;(void)t;return ST_INVALID;}
int st_leave(struct st_world *w,unsigned c,struct sl_stats *t){(void)w;(void)c;(void)t;return ST_INVALID;}
int st_begin(struct st_world *w,unsigned a,unsigned s,uintptr_t f,unsigned n,unsigned k,struct st_request *r,struct sl_stats *t){(void)w;(void)a;(void)s;(void)f;(void)n;(void)k;(void)r;(void)t;return ST_INVALID;}
int st_handle(struct st_world *w,unsigned c,struct st_request r){(void)w;(void)c;(void)r;return ST_INVALID;}
int st_request(const struct st_world *w,unsigned s,struct st_request *r){(void)w;(void)s;(void)r;return ST_INVALID;}
int st_retry(struct st_world *w,struct st_request r,struct sl_stats *t){(void)w;(void)r;(void)t;return ST_INVALID;}
int st_finish(struct st_world *w,struct st_request r,int e,struct st_permit *p,struct sl_stats *t){(void)w;(void)r;(void)e;(void)p;(void)t;return ST_INVALID;}
