#include "cell_guard.h"
int ocg_init(OcgCell *c,int32_t v){(void)c;(void)v;return 1;}
OwStatus ocg_acquire(OcgCell *c,int w,OcgToken *t){(void)c;(void)w;(void)t;return OW_NOT_IMPLEMENTED;}
OwStatus ocg_release(OcgCell *c,OcgToken t){(void)c;(void)t;return OW_NOT_IMPLEMENTED;}
OwStatus ocg_read(OcgCell *c,OcgToken t,int32_t *o){(void)c;(void)t;(void)o;return OW_NOT_IMPLEMENTED;}
OwStatus ocg_write(OcgCell *c,OcgToken t,int32_t v){(void)c;(void)t;(void)v;return OW_NOT_IMPLEMENTED;}
OwStatus ocg_replace(OcgCell *c,int32_t v,int32_t *o){(void)c;(void)v;(void)o;return OW_NOT_IMPLEMENTED;}
OwStatus ocg_update(OcgCell *c,OcgUpdate f,void *p,int32_t *o){(void)c;(void)f;(void)p;(void)o;return OW_NOT_IMPLEMENTED;}
OwStatus ocg_close(OcgCell *c){(void)c;return OW_NOT_IMPLEMENTED;}
OwStatus ocg_destroy(OcgCell *c){(void)c;return OW_NOT_IMPLEMENTED;}
