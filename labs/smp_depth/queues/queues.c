#include "queues.h"
int sq_init(SqWorld *w,unsigned n,unsigned c,unsigned l){(void)w;(void)n;(void)c;(void)l;return SQ_INVALID;}
int sq_create(SqWorld *w,unsigned a,unsigned q,unsigned s,SqHandle *h,SqEvent *e){(void)w;(void)a;(void)q;(void)s;(void)h;(void)e;return SQ_INVALID;}
int sq_dispatch(SqWorld *w,unsigned c,SqHandle *h,SqEvent *e,SlStats *s){(void)w;(void)c;(void)h;(void)e;(void)s;return SQ_INVALID;}
int sq_finish(SqWorld *w,unsigned c,SqHandle h,unsigned n,SqEvent *e,SlStats *s){(void)w;(void)c;(void)h;(void)n;(void)e;(void)s;return SQ_INVALID;}
int sq_wake(SqWorld *w,unsigned a,unsigned q,SqHandle h,SqEvent *e,SlStats *s){(void)w;(void)a;(void)q;(void)h;(void)e;(void)s;return SQ_INVALID;}
int sq_transfer_begin(SqWorld *w,unsigned a,unsigned x,unsigned y,SqHandle h,SqTransfer *t,SqEvent *e,SlStats *s){(void)w;(void)a;(void)x;(void)y;(void)h;(void)t;(void)e;(void)s;return SQ_INVALID;}
int sq_transfer_end(SqWorld *w,unsigned a,SqTransfer t,int c,SqEvent *e,SlStats *s){(void)w;(void)a;(void)t;(void)c;(void)e;(void)s;return SQ_INVALID;}
int sq_invariant(const SqWorld *w){(void)w;return 0;}
