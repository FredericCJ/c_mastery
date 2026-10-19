#include "power.h"
enum cs_status cs_stop_init(CSStop*s){(void)s;return CS_UNIMPLEMENTED;}
enum cs_status cs_stop_enter(CSStop*s,const CSBus*b,unsigned p){(void)s;(void)b;(void)p;return CS_UNIMPLEMENTED;}
enum cs_status cs_stop_finish(CSStop*s,const CSBus*b,unsigned k,uint64_t x,uint64_t y){(void)s;(void)b;(void)k;(void)x;(void)y;return CS_UNIMPLEMENTED;}
enum cs_status cs_iwdg_start(const CSBus*b,unsigned p,unsigned r,unsigned n){(void)b;(void)p;(void)r;(void)n;return CS_UNIMPLEMENTED;}
enum cs_status cs_iwdg_feed(CSWorld*w,const CSBus*b){(void)w;(void)b;return CS_UNIMPLEMENTED;}
