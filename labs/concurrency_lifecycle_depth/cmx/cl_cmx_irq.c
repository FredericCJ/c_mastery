#include "cl_cmx_irq.h"
/* Learner target port scaffold. */
int cc_irq_receive(struct cc_handle h,size_t n,cmx_deadline d,struct cc_ticket *t){(void)h;(void)n;(void)d;(void)t;return CL_UNIMPLEMENTED;}
int cc_irq_publish(struct cc_port p,uint32_t a,uint32_t s,const unsigned char *b,size_t n){(void)p;(void)a;(void)s;(void)b;(void)n;return CL_UNIMPLEMENTED;}
int cc_irq_join0(struct cc_port *p){(void)p;return CL_UNIMPLEMENTED;}
