#include "release.h"
int dt_emit(DtTrace*q,uint32_t k,uint32_t t,uint32_t p){(void)q;(void)k;(void)t;(void)p;return DT_UNIMPLEMENTED;}
int dt_take(DtTrace*q,DtRecord*r){(void)q;(void)r;return DT_UNIMPLEMENTED;}
int dt_irq_emit(DtTrace*q,DtCritical c,uint32_t k,uint32_t t,uint32_t p){(void)q;(void)c;(void)k;(void)t;(void)p;return DT_UNIMPLEMENTED;}
int dt_irq_take(DtTrace*q,DtCritical c,DtRecord*r){(void)q;(void)c;(void)r;return DT_UNIMPLEMENTED;}
int dt_duration(const DtInterval*i,size_t n,DtDuration*r){(void)i;(void)n;(void)r;return DT_UNIMPLEMENTED;}
int dt_image(uint32_t b,uint32_t n,uint32_t v,DtImage*r){(void)b;(void)n;(void)v;(void)r;return DT_UNIMPLEMENTED;}
int dt_startup(DtRam*r,const unsigned char i[8],unsigned w,uint32_t id){(void)r;(void)i;(void)w;(void)id;return DT_UNIMPLEMENTED;}
int dt_record(DtRetained*r,uint32_t i,uint32_t c){(void)r;(void)i;(void)c;return DT_UNIMPLEMENTED;}
