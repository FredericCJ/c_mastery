#include "phy.h"
int ep_write(const EDBus *a,uint32_t b,unsigned c,unsigned d,unsigned e,uint32_t f,uint16_t g){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;return ED_UNIMPLEMENTED;}
int ep_find(const EDBus *a,uint32_t b,unsigned c,uint32_t d,unsigned *e){(void)a;(void)b;(void)c;(void)d;(void)e;return ED_UNIMPLEMENTED;}
int ep_init(EpPhy *a,const EDBus *b,uint32_t c,unsigned d,unsigned e,unsigned f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return ED_UNIMPLEMENTED;}
int ep_begin(EpPhy *a,unsigned b,uint32_t c){(void)a;(void)b;(void)c;return ED_UNIMPLEMENTED;}
int ep_step(EpPhy *a){(void)a;return ED_UNIMPLEMENTED;}
int ep_poll(EpPhy *a){(void)a;return ED_UNIMPLEMENTED;}
int ep_invalidate(EpPhy *a){(void)a;return ED_UNIMPLEMENTED;}
int ep_snapshot(const EpPhy *a,EpLink *b){(void)a;(void)b;return ED_UNIMPLEMENTED;}
int ep_validate(const EpPhy *a,const EpLink *b){(void)a;(void)b;return ED_UNIMPLEMENTED;}
