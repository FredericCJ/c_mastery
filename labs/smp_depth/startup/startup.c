#include "startup.h"
/* Implement the finite generation and resource-ownership protocol. */
int ss_begin(struct ss_world *w,unsigned s,unsigned i,unsigned a,uint64_t p,uint64_t lo,uint64_t hi,struct ss_ticket *t){(void)w;(void)s;(void)i;(void)a;(void)p;(void)lo;(void)hi;(void)t;return SS_INVALID;}
int ss_timeout(struct ss_world *w,struct ss_ticket t){(void)w;(void)t;return SS_INVALID;}
int ss_ack(struct ss_world *w,struct ss_ticket t,struct ss_evidence e){(void)w;(void)t;(void)e;return SS_INVALID;}
int ss_quiescent(struct ss_world *w,struct ss_ticket t,int proof){(void)w;(void)t;(void)proof;return SS_INVALID;}
int ss_release(struct ss_world *w,struct ss_ticket t){(void)w;(void)t;return SS_INVALID;}
