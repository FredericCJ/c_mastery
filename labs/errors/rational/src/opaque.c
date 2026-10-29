#include "opaque.h"
er_code efr_opaque_make(struct efr_memory m,uint32_t n,uint32_t d,struct efr_opaque **o){(void)m;(void)n;(void)d;(void)o;return ER_NOT_IMPLEMENTED;}
er_code efr_opaque_consume(const struct efr_opaque *p,uint32_t *o){(void)p;(void)o;return ER_NOT_IMPLEMENTED;}
void efr_opaque_drop(struct efr_opaque **p){(void)p;}
