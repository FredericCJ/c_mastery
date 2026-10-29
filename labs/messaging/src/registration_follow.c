#include "registration.h"
enum msg_status mfr_init(struct mfr_hub *h,size_t a,size_t b) {(void)h;(void)a;(void)b;return MSG_FAILED;}
enum msg_status mfr_send(struct mfr_hub *h,unsigned k,unsigned v) {(void)h;(void)k;(void)v;return MSG_FAILED;}
enum msg_status mfr_select(struct mfr_hub *h,unsigned m,bool w,unsigned *k,unsigned *v) {(void)h;(void)m;(void)w;(void)k;(void)v;return MSG_FAILED;}
enum msg_status mfr_select_probe(struct mfr_hub *h,unsigned m,unsigned *k,unsigned *v,const struct timespec *d,mfr_probe_fn p,void *c) {(void)h;(void)m;(void)k;(void)v;(void)d;(void)p;(void)c;return MSG_FAILED;}
enum msg_status mfr_close(struct mfr_hub *h,unsigned k) {(void)h;(void)k;return MSG_FAILED;}
void mfr_cancel(struct mfr_hub *h) {(void)h;}
void mfr_observe(struct mfr_hub *h,size_t *w,uint64_t *s) {(void)h;*w=0;*s=0;}
enum msg_status mfr_destroy(struct mfr_hub *h) {(void)h;return MSG_FAILED;}
