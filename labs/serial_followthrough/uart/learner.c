#include "uart_depth.h"
int ua_format(uint32_t a,uint32_t b,unsigned c,unsigned d,unsigned e,unsigned f,uint16_t g,struct ua_format *h){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;return PU_UNIMPLEMENTED;}
int ua_apply(struct pu_driver *a,struct pu_io b,const struct ua_format *c){(void)a;(void)b;(void)c;return PU_UNIMPLEMENTED;}
int ua_begin(struct ua_tx *a,struct pu_io b,uint32_t c,uint32_t d,const uint8_t *e,unsigned f,uint32_t g,uint32_t h){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;return PU_UNIMPLEMENTED;}
int ua_step(struct ua_tx *a,struct pu_io b,uint32_t c,uint32_t d){(void)a;(void)b;(void)c;(void)d;return PU_UNIMPLEMENTED;}
int ua_cancel(struct ua_tx *a,struct pu_io b,uint32_t c){(void)a;(void)b;(void)c;return PU_UNIMPLEMENTED;}
int ua_rx(struct ua_rx *a,struct pu_io b){(void)a;(void)b;return PU_UNIMPLEMENTED;}
int ua_take(struct ua_rx *a,struct pu_byte *b){(void)a;(void)b;return PU_UNIMPLEMENTED;}
