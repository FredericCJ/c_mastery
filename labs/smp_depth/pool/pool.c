#include "pool.h"
/* Learner implementation. Every stub compiles but supplies no accepted work. */
int sp_init(struct sp_pool *p,uint64_t w,unsigned n,int d){(void)p;(void)w;(void)n;(void)d;return SP_INVALID;}
int sp_submit(struct sp_pool *p,unsigned h,uint64_t v,uint64_t c,struct sp_id *o){(void)p;(void)h;(void)v;(void)c;(void)o;return SP_INVALID;}
int sp_claim(struct sp_pool *p,unsigned w,struct sp_claim *o){(void)p;(void)w;(void)o;return SP_INVALID;}
int sp_complete(struct sp_pool *p,struct sp_claim c,uint64_t v){(void)p;(void)c;(void)v;return SP_INVALID;}
int sp_cancel(struct sp_pool *p,struct sp_id i){(void)p;(void)i;return SP_INVALID;}
int sp_result(struct sp_pool *p,struct sp_id i,unsigned *s,uint64_t *v){(void)p;(void)i;(void)s;(void)v;return SP_INVALID;}
int sp_collect(struct sp_pool *p,struct sp_id i){(void)p;(void)i;return SP_INVALID;}
int sp_close(struct sp_pool *p,int c){(void)p;(void)c;return SP_INVALID;}
int sp_snapshot(struct sp_pool *p,struct sp_snapshot *o){(void)p;(void)o;return SP_INVALID;}
int sp_launch_init(struct sp_launch *p,struct sp_launch_port q){(void)p;(void)q;return SP_INVALID;}
int sp_launch(struct sp_launch *p,unsigned n){(void)p;(void)n;return SP_INVALID;}
int sp_shutdown(struct sp_launch *p){(void)p;return SP_INVALID;}
