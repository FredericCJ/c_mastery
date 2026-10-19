#include "registration.h"
MsgStatus mfr_init(MfrHub *h,size_t a,size_t b) {(void)h;(void)a;(void)b;return MSG_FAILED;}
MsgStatus mfr_send(MfrHub *h,unsigned k,unsigned v) {(void)h;(void)k;(void)v;return MSG_FAILED;}
MsgStatus mfr_select(MfrHub *h,unsigned m,bool w,unsigned *k,unsigned *v) {(void)h;(void)m;(void)w;(void)k;(void)v;return MSG_FAILED;}
MsgStatus mfr_select_probe(MfrHub *h,unsigned m,unsigned *k,unsigned *v,const struct timespec *d,MfrProbe p,void *c) {(void)h;(void)m;(void)k;(void)v;(void)d;(void)p;(void)c;return MSG_FAILED;}
MsgStatus mfr_close(MfrHub *h,unsigned k) {(void)h;(void)k;return MSG_FAILED;}
void mfr_cancel(MfrHub *h) {(void)h;}
void mfr_observe(MfrHub *h,size_t *w,uint64_t *s) {(void)h;*w=0;*s=0;}
MsgStatus mfr_destroy(MfrHub *h) {(void)h;return MSG_FAILED;}
