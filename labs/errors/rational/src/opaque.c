#include "opaque.h"
er_code efr_opaque_make(EfrMemory m,uint32_t n,uint32_t d,EfrOpaque **o){(void)m;(void)n;(void)d;(void)o;return ER_NOT_IMPLEMENTED;}
er_code efr_opaque_consume(const EfrOpaque *p,uint32_t *o){(void)p;(void)o;return ER_NOT_IMPLEMENTED;}
void efr_opaque_drop(EfrOpaque **p){(void)p;}
