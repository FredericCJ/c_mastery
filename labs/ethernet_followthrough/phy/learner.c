#include "phy.h"
int ep_write(const struct ed_bus *a,uint32_t b,unsigned c,unsigned d,unsigned e,uint32_t f,uint16_t g){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;return ED_UNIMPLEMENTED;}
int ep_find(const struct ed_bus *a,uint32_t b,unsigned c,uint32_t d,unsigned *e){(void)a;(void)b;(void)c;(void)d;(void)e;return ED_UNIMPLEMENTED;}
int ep_init(struct ep_phy *a,const struct ed_bus *b,uint32_t c,unsigned d,unsigned e,unsigned f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return ED_UNIMPLEMENTED;}
int ep_begin(struct ep_phy *a,unsigned b,uint32_t c){(void)a;(void)b;(void)c;return ED_UNIMPLEMENTED;}
int ep_step(struct ep_phy *a){(void)a;return ED_UNIMPLEMENTED;}
int ep_poll(struct ep_phy *a){(void)a;return ED_UNIMPLEMENTED;}
int ep_invalidate(struct ep_phy *a){(void)a;return ED_UNIMPLEMENTED;}
int ep_snapshot(const struct ep_phy *a,struct ep_link *b){(void)a;(void)b;return ED_UNIMPLEMENTED;}
int ep_validate(const struct ep_phy *a,const struct ep_link *b){(void)a;(void)b;return ED_UNIMPLEMENTED;}
