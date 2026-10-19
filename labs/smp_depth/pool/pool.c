#include "pool.h"
/* Learner implementation. Every stub compiles but supplies no accepted work. */
int sp_init(SpPool *p,uint64_t w,unsigned n,int d){(void)p;(void)w;(void)n;(void)d;return SP_INVALID;}
int sp_submit(SpPool *p,unsigned h,uint64_t v,uint64_t c,SpId *o){(void)p;(void)h;(void)v;(void)c;(void)o;return SP_INVALID;}
int sp_claim(SpPool *p,unsigned w,SpClaim *o){(void)p;(void)w;(void)o;return SP_INVALID;}
int sp_complete(SpPool *p,SpClaim c,uint64_t v){(void)p;(void)c;(void)v;return SP_INVALID;}
int sp_cancel(SpPool *p,SpId i){(void)p;(void)i;return SP_INVALID;}
int sp_result(SpPool *p,SpId i,unsigned *s,uint64_t *v){(void)p;(void)i;(void)s;(void)v;return SP_INVALID;}
int sp_collect(SpPool *p,SpId i){(void)p;(void)i;return SP_INVALID;}
int sp_close(SpPool *p,int c){(void)p;(void)c;return SP_INVALID;}
int sp_snapshot(SpPool *p,SpSnapshot *o){(void)p;(void)o;return SP_INVALID;}
int sp_launch_init(SpLaunch *p,SpLaunchPort q){(void)p;(void)q;return SP_INVALID;}
int sp_launch(SpLaunch *p,unsigned n){(void)p;(void)n;return SP_INVALID;}
int sp_shutdown(SpLaunch *p){(void)p;return SP_INVALID;}
