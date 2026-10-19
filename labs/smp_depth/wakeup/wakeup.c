#include "wakeup.h"
int sw_init(SwWorld *w,unsigned n,unsigned c,SwPort p){(void)w;(void)n;(void)c;(void)p;return SQ_INVALID;}
int sw_create(SwWorld *w,unsigned a,unsigned h,unsigned x,SqHandle *i,SqEvent *e){(void)w;(void)a;(void)h;(void)x;(void)i;(void)e;return SQ_INVALID;}
int sw_wake(SwWorld *w,unsigned a,unsigned h,SqHandle i,SqEvent *e,SlStats *s){(void)w;(void)a;(void)h;(void)i;(void)e;(void)s;return SQ_INVALID;}
int sw_dispatch(SwWorld *w,unsigned c,SqHandle *i,SqEvent *e,SlStats *s){(void)w;(void)c;(void)i;(void)e;(void)s;return SQ_INVALID;}
int sw_finish(SwWorld *w,unsigned c,SqHandle i,unsigned n,SqEvent *e,SlStats *s){(void)w;(void)c;(void)i;(void)n;(void)e;(void)s;return SQ_INVALID;}
int sw_transfer_begin(SwWorld *w,unsigned a,unsigned x,unsigned y,SqHandle i,SqTransfer *t,SqEvent *e,SlStats *s){(void)w;(void)a;(void)x;(void)y;(void)i;(void)t;(void)e;(void)s;return SQ_INVALID;}
int sw_transfer_end(SwWorld *w,unsigned a,SqTransfer t,int c,SqEvent *e,SlStats *s){(void)w;(void)a;(void)t;(void)c;(void)e;(void)s;return SQ_INVALID;}
int sw_retry(SwWorld *w,unsigned a,unsigned d,SlStats *s){(void)w;(void)a;(void)d;(void)s;return SQ_INVALID;}
int sw_prepare_idle(SwWorld *w,unsigned c,int m,SlStats *s){(void)w;(void)c;(void)m;(void)s;return SQ_INVALID;}
