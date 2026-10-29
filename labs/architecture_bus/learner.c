#include "bus.h"
int mb_map(unsigned a,unsigned b,struct mb_map*c){(void)a;(void)b;(void)c;return MB_INVALID;}
int mb_place(unsigned a,unsigned b,unsigned c,unsigned d,uint32_t e,uint32_t f,unsigned g,uint32_t*h){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;return MB_INVALID;}
int mb_bind(struct mb_lease*a,unsigned char*b,unsigned char*c,uint32_t d,uint32_t e,unsigned f,uint32_t g,unsigned h){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;return PD_INVALID;}
int mb_give(struct mb_lease*a,struct pd_io b,uint32_t c){(void)a;(void)b;(void)c;return PD_INVALID;}
int mb_cpu_write(struct mb_lease*a,uint32_t b,unsigned char c){(void)a;(void)b;(void)c;return PD_INVALID;}
int mb_cpu_read(const struct mb_lease*a,uint32_t b,unsigned char*c){(void)a;(void)b;(void)c;return PD_INVALID;}
int mb_poll(struct mb_lease*a,struct pd_io b,uint32_t c){(void)a;(void)b;(void)c;return PD_INVALID;}
int mb_finish(struct mb_lease*a,struct pd_io b,uint32_t c,struct pd_result*d){(void)a;(void)b;(void)c;(void)d;return PD_INVALID;}
int mb_arbitrate(struct mb_bus_config a,struct mb_bus_trace*b){(void)a;(void)b;return MB_INVALID;}
int mb_bridge(unsigned a,uint32_t b,uint32_t*c){(void)a;(void)b;(void)c;return MB_INVALID;}
int mb_usart_width(unsigned a){(void)a;return 0;}
