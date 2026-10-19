#include "adapters.h"
OwStatus cpx_owner_close(CPXOwner *o,int (*u)(void *,uint64_t),void *c){(void)o;(void)u;(void)c;return OW_NOT_IMPLEMENTED;}
OwStatus cpx_owner_ack(CPXOwner *o,uint64_t c,int q){(void)o;(void)c;(void)q;return OW_NOT_IMPLEMENTED;}
er_code cpx_progress_step(CPXProgress *p,const unsigned char *b,size_t n,CPWWrite w,void *c){(void)p;(void)b;(void)n;(void)w;(void)c;return ER_NOT_IMPLEMENTED;}
void cpx_progress_cleanup(CPXProgress *p,int c){(void)p;(void)c;}
