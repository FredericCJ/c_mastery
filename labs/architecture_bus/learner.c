#include "bus.h"
int mb_map(unsigned a,unsigned b,MBMap*c){(void)a;(void)b;(void)c;return MB_INVALID;}
int mb_place(unsigned a,unsigned b,unsigned c,unsigned d,uint32_t e,uint32_t f,unsigned g,uint32_t*h){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;return MB_INVALID;}
int mb_bind(MBLease*a,unsigned char*b,unsigned char*c,uint32_t d,uint32_t e,unsigned f,uint32_t g,unsigned h){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;return PD_INVALID;}
int mb_give(MBLease*a,PdIo b,uint32_t c){(void)a;(void)b;(void)c;return PD_INVALID;}
int mb_cpu_write(MBLease*a,uint32_t b,unsigned char c){(void)a;(void)b;(void)c;return PD_INVALID;}
int mb_cpu_read(const MBLease*a,uint32_t b,unsigned char*c){(void)a;(void)b;(void)c;return PD_INVALID;}
int mb_poll(MBLease*a,PdIo b,uint32_t c){(void)a;(void)b;(void)c;return PD_INVALID;}
int mb_finish(MBLease*a,PdIo b,uint32_t c,PdResult*d){(void)a;(void)b;(void)c;(void)d;return PD_INVALID;}
int mb_arbitrate(MBBusConfig a,MBBusTrace*b){(void)a;(void)b;return MB_INVALID;}
int mb_bridge(unsigned a,uint32_t b,uint32_t*c){(void)a;(void)b;(void)c;return MB_INVALID;}
int mb_usart_width(unsigned a){(void)a;return 0;}
