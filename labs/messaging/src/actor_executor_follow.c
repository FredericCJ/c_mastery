#include "actor_executor.h"
MsgStatus mfe_init(MfeExecutor *x,size_t a,size_t b,MfaHandler h,void *c[2],MfaDeadLetter d,void *p) {(void)x;(void)a;(void)b;(void)h;(void)c;(void)d;(void)p;return MSG_FAILED;}
MsgStatus mfe_send(MfeExecutor *x,unsigned d,uint64_t g,MfoValue *v,bool w) {(void)x;(void)d;(void)g;(void)v;(void)w;return MSG_FAILED;}
MsgStatus mfe_pump(MfeExecutor *x,bool w,MfeEvent *e) {(void)x;(void)w;(void)e;return MSG_FAILED;}
MsgStatus mfe_generation(MfeExecutor *x,unsigned d,uint64_t *g) {(void)x;(void)d;(void)g;return MSG_FAILED;}
MsgStatus mfe_restart(MfeExecutor *x,unsigned d) {(void)x;(void)d;return MSG_FAILED;}
MsgStatus mfe_close(MfeExecutor *x,unsigned d) {(void)x;(void)d;return MSG_FAILED;}
void mfe_cancel(MfeExecutor *x) {(void)x;}
MsgStatus mfe_destroy(MfeExecutor *x) {(void)x;return MSG_FAILED;}
