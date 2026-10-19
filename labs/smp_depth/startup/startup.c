#include "startup.h"
/* Implement the finite generation and resource-ownership protocol. */
int ss_begin(SsWorld *w,unsigned s,unsigned i,unsigned a,uint64_t p,uint64_t lo,uint64_t hi,SsTicket *t){(void)w;(void)s;(void)i;(void)a;(void)p;(void)lo;(void)hi;(void)t;return SS_INVALID;}
int ss_timeout(SsWorld *w,SsTicket t){(void)w;(void)t;return SS_INVALID;}
int ss_ack(SsWorld *w,SsTicket t,SsEvidence e){(void)w;(void)t;(void)e;return SS_INVALID;}
int ss_quiescent(SsWorld *w,SsTicket t,int proof){(void)w;(void)t;(void)proof;return SS_INVALID;}
int ss_release(SsWorld *w,SsTicket t){(void)w;(void)t;return SS_INVALID;}
