#include "cl_cmx_task.h"
/* Learner synchronous task-port scaffold; retain explicit runtime imports. */
int cc_task_send(CCHandle h,uint32_t p,uint32_t s,const unsigned char *b,size_t n,cmx_deadline d){(void)h;(void)p;(void)s;(void)b;(void)n;(void)d;return CL_UNIMPLEMENTED;}
int cc_task_receive(CCHandle h,size_t n,cmx_deadline d,CLMessage *m){(void)h;(void)n;(void)d;(void)m;return CL_UNIMPLEMENTED;}
int cc_task_condition(CCHandle h,cmx_cond *c,cmx_mutex *m,cmx_deadline d){(void)h;(void)c;(void)m;(void)d;return CL_UNIMPLEMENTED;}
int cc_task_condition_unlock(CCHandle h){(void)h;return CL_UNIMPLEMENTED;}
int cc_task_close(CCQueue *q,unsigned a){(void)q;(void)a;return CL_UNIMPLEMENTED;}
