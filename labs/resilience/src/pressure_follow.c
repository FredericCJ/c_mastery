#include "pressure.h"
void rzp_owned_init(RzpOwnedQueue *p){if(p)*p=(RzpOwnedQueue){0};}
void rzp_value_drop(RzpValue *p){(void)p;}
int rzp_owned_offer(RzpOwnedQueue *p,const RZScope *s,uint64_t n,RZPolicy x,RzpValue *v){(void)p;(void)s;(void)n;(void)x;(void)v;return RZ_INVALID;}
int rzp_owned_pop(RzpOwnedQueue *p,RzpValue *v){(void)p;(void)v;return RZ_INVALID;}
void rzp_owned_close(RzpOwnedQueue *p){(void)p;}
int rzp_init(RzpPump *p,size_t n){(void)p;(void)n;return RZ_INVALID;}
int rzp_begin(RzpPump *p,unsigned i,RzfScope *s,uint64_t n,RzpValue *v,RzpTicket *t){(void)p;(void)i;(void)s;(void)n;(void)v;(void)t;return RZ_INVALID;}
int rzp_poll(RzpPump *p,RzpTicket t,bool w,int *v){(void)p;(void)t;(void)w;(void)v;return RZ_INVALID;}
int rzp_cancel(RzpPump *p,RzpTicket t){(void)p;(void)t;return RZ_INVALID;}
int rzp_tick(RzpPump *p,uint64_t n){(void)p;(void)n;return RZ_INVALID;}
int rzp_forward(RzpPump *p){(void)p;return RZ_INVALID;}
int rzp_receive(RzpPump *p,RzpValue *v,RzpCredit *c){(void)p;(void)v;(void)c;return RZ_INVALID;}
int rzp_release(RzpPump *p,RzpCredit c){(void)p;(void)c;return RZ_INVALID;}
int rzp_close(RzpPump *p){(void)p;return RZ_INVALID;}
int rzp_destroy(RzpPump *p){(void)p;return RZ_INVALID;}
int rzp_snapshot(RzpPump *p,RzpSnapshot *s){(void)p;(void)s;return RZ_INVALID;}
