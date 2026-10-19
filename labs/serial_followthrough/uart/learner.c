#include "uart_depth.h"
int ua_format(uint32_t a,uint32_t b,unsigned c,unsigned d,unsigned e,unsigned f,uint16_t g,UaFormat *h){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;return PU_UNIMPLEMENTED;}
int ua_apply(PuDriver *a,PuIo b,const UaFormat *c){(void)a;(void)b;(void)c;return PU_UNIMPLEMENTED;}
int ua_begin(UaTx *a,PuIo b,uint32_t c,uint32_t d,const uint8_t *e,unsigned f,uint32_t g,uint32_t h){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;return PU_UNIMPLEMENTED;}
int ua_step(UaTx *a,PuIo b,uint32_t c,uint32_t d){(void)a;(void)b;(void)c;(void)d;return PU_UNIMPLEMENTED;}
int ua_cancel(UaTx *a,PuIo b,uint32_t c){(void)a;(void)b;(void)c;return PU_UNIMPLEMENTED;}
int ua_rx(UaRx *a,PuIo b){(void)a;(void)b;return PU_UNIMPLEMENTED;}
int ua_take(UaRx *a,PuByte *b){(void)a;(void)b;return PU_UNIMPLEMENTED;}
