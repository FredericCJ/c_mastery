#include "actor_executor.h"
enum msg_status mfe_init(struct mfe_executor *x,size_t a,size_t b,mfa_handler_fn h,void *c[2],mfa_dead_letter_fn d,void *p) {(void)x;(void)a;(void)b;(void)h;(void)c;(void)d;(void)p;return MSG_FAILED;}
enum msg_status mfe_send(struct mfe_executor *x,unsigned d,uint64_t g,struct mfo_value *v,bool w) {(void)x;(void)d;(void)g;(void)v;(void)w;return MSG_FAILED;}
enum msg_status mfe_pump(struct mfe_executor *x,bool w,struct mfe_event *e) {(void)x;(void)w;(void)e;return MSG_FAILED;}
enum msg_status mfe_generation(struct mfe_executor *x,unsigned d,uint64_t *g) {(void)x;(void)d;(void)g;return MSG_FAILED;}
enum msg_status mfe_restart(struct mfe_executor *x,unsigned d) {(void)x;(void)d;return MSG_FAILED;}
enum msg_status mfe_close(struct mfe_executor *x,unsigned d) {(void)x;(void)d;return MSG_FAILED;}
void mfe_cancel(struct mfe_executor *x) {(void)x;}
enum msg_status mfe_destroy(struct mfe_executor *x) {(void)x;return MSG_FAILED;}
