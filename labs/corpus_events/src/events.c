#include "events.h"
enum ev_status ev_init(struct ev_parser*p,size_t n){(void)p;(void)n;return EV_INVALID;}
enum ev_status ev_feed(struct ev_parser*p,const unsigned char*b,size_t n,ev_handler_fn h,void*c,size_t*u){(void)p;(void)b;(void)n;(void)h;(void)c;(void)u;return EV_INVALID;}
enum ev_status ev_finish(struct ev_parser*p,ev_handler_fn h,void*c){(void)p;(void)h;(void)c;return EV_INVALID;}
enum ev_status ev_cancel(struct ev_parser*p){(void)p;return EV_INVALID;}
void evq_init(struct ev_queue*q){(void)q;}
enum ev_action evq_offer(void*c,const struct ev_event*e){(void)c;(void)e;return EV_REJECT;}
enum ev_status evq_pop(struct ev_queue*q,struct ev_event*e){(void)q;(void)e;return EV_INVALID;}
void evs_init(struct ev_sum*s){(void)s;}
enum ev_status evs_event(struct ev_sum*s,const struct ev_event*e){(void)s;(void)e;return EV_INVALID;}
