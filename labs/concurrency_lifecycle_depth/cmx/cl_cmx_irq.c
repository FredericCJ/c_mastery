#include "cl_cmx_irq.h"
/* Learner target port scaffold. */
int cc_irq_receive(CCHandle h,size_t n,cmx_deadline d,CCTicket *t){(void)h;(void)n;(void)d;(void)t;return CL_UNIMPLEMENTED;}
int cc_irq_publish(CCPort p,uint32_t a,uint32_t s,const unsigned char *b,size_t n){(void)p;(void)a;(void)s;(void)b;(void)n;return CL_UNIMPLEMENTED;}
int cc_irq_join0(CCPort *p){(void)p;return CL_UNIMPLEMENTED;}
