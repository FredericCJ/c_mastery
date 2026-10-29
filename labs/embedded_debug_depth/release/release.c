#include "release.h"
int dt_emit(struct dt_trace*q,uint32_t k,uint32_t t,uint32_t p){(void)q;(void)k;(void)t;(void)p;return DT_UNIMPLEMENTED;}
int dt_take(struct dt_trace*q,struct dt_record*r){(void)q;(void)r;return DT_UNIMPLEMENTED;}
int dt_irq_emit(struct dt_trace*q,struct dt_critical c,uint32_t k,uint32_t t,uint32_t p){(void)q;(void)c;(void)k;(void)t;(void)p;return DT_UNIMPLEMENTED;}
int dt_irq_take(struct dt_trace*q,struct dt_critical c,struct dt_record*r){(void)q;(void)c;(void)r;return DT_UNIMPLEMENTED;}
int dt_duration(const struct dt_interval*i,size_t n,struct dt_duration*r){(void)i;(void)n;(void)r;return DT_UNIMPLEMENTED;}
int dt_image(uint32_t b,uint32_t n,uint32_t v,struct dt_image*r){(void)b;(void)n;(void)v;(void)r;return DT_UNIMPLEMENTED;}
int dt_startup(struct dt_ram*r,const unsigned char i[8],unsigned w,uint32_t id){(void)r;(void)i;(void)w;(void)id;return DT_UNIMPLEMENTED;}
int dt_record(struct dt_retained*r,uint32_t i,uint32_t c){(void)r;(void)i;(void)c;return DT_UNIMPLEMENTED;}
