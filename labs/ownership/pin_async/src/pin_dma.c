#include "pin_dma.h"
enum ow_status opa_dma_init(struct opa_dma *d,const void *s,struct pd_io i){(void)d;(void)s;(void)i;return OW_NOT_IMPLEMENTED;}
enum ow_status opa_dma_submit(struct opa_dma *d,const struct pd_extent *e,unsigned n,unsigned b,unsigned c,uint32_t *t){(void)d;(void)e;(void)n;(void)b;(void)c;(void)t;return OW_NOT_IMPLEMENTED;}
enum ow_status opa_dma_timeout(struct opa_dma *d,uint32_t t){(void)d;(void)t;return OW_NOT_IMPLEMENTED;}
enum ow_status opa_dma_poll(struct opa_dma *d,uint32_t t,struct pd_result *r){(void)d;(void)t;(void)r;return OW_NOT_IMPLEMENTED;}
