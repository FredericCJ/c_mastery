#include "adapters.h"
enum ow_status cpx_owner_close(struct cpx_owner *o,int (*u)(void *,uint64_t),void *c){(void)o;(void)u;(void)c;return OW_NOT_IMPLEMENTED;}
enum ow_status cpx_owner_ack(struct cpx_owner *o,uint64_t c,int q){(void)o;(void)c;(void)q;return OW_NOT_IMPLEMENTED;}
er_code cpx_progress_step(struct cpx_progress *p,const unsigned char *b,size_t n,cpw_write_fn w,void *c){(void)p;(void)b;(void)n;(void)w;(void)c;return ER_NOT_IMPLEMENTED;}
void cpx_progress_cleanup(struct cpx_progress *p,int c){(void)p;(void)c;}
