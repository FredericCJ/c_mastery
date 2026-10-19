#include "pin_dma.h"
OwStatus opa_dma_init(OpaDma *d,const void *s,PdIo i){(void)d;(void)s;(void)i;return OW_NOT_IMPLEMENTED;}
OwStatus opa_dma_submit(OpaDma *d,const PdExtent *e,unsigned n,unsigned b,unsigned c,uint32_t *t){(void)d;(void)e;(void)n;(void)b;(void)c;(void)t;return OW_NOT_IMPLEMENTED;}
OwStatus opa_dma_timeout(OpaDma *d,uint32_t t){(void)d;(void)t;return OW_NOT_IMPLEMENTED;}
OwStatus opa_dma_poll(OpaDma *d,uint32_t t,PdResult *r){(void)d;(void)t;(void)r;return OW_NOT_IMPLEMENTED;}
