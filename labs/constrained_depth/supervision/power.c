#include "power.h"
enum cs_status cs_stop_init(struct cs_stop*s){(void)s;return CS_UNIMPLEMENTED;}
enum cs_status cs_stop_enter(struct cs_stop*s,const struct cs_bus*b,unsigned p){(void)s;(void)b;(void)p;return CS_UNIMPLEMENTED;}
enum cs_status cs_stop_finish(struct cs_stop*s,const struct cs_bus*b,unsigned k,uint64_t x,uint64_t y){(void)s;(void)b;(void)k;(void)x;(void)y;return CS_UNIMPLEMENTED;}
enum cs_status cs_iwdg_start(const struct cs_bus*b,unsigned p,unsigned r,unsigned n){(void)b;(void)p;(void)r;(void)n;return CS_UNIMPLEMENTED;}
enum cs_status cs_iwdg_feed(struct cs_world*w,const struct cs_bus*b){(void)w;(void)b;return CS_UNIMPLEMENTED;}
