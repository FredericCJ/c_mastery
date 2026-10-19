#include "events.h"
EvStatus ev_init(EvParser*p,size_t n){(void)p;(void)n;return EV_INVALID;}
EvStatus ev_feed(EvParser*p,const unsigned char*b,size_t n,EvHandler h,void*c,size_t*u){(void)p;(void)b;(void)n;(void)h;(void)c;(void)u;return EV_INVALID;}
EvStatus ev_finish(EvParser*p,EvHandler h,void*c){(void)p;(void)h;(void)c;return EV_INVALID;}
EvStatus ev_cancel(EvParser*p){(void)p;return EV_INVALID;}
void evq_init(EvQueue*q){(void)q;}
EvAction evq_offer(void*c,const EvEvent*e){(void)c;(void)e;return EV_REJECT;}
EvStatus evq_pop(EvQueue*q,EvEvent*e){(void)q;(void)e;return EV_INVALID;}
void evs_init(EvSum*s){(void)s;}
EvStatus evs_event(EvSum*s,const EvEvent*e){(void)s;(void)e;return EV_INVALID;}
