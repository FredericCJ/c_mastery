#include "dod_entities.h"
void d_entity_init(struct d_entity_world *w){(void)w;}
int d_entity_create(struct d_entity_world *w,unsigned m,int p,int v,struct d_handle *out){(void)w;(void)m;(void)p;(void)v;(void)out;return D_FAILED;}
int d_entity_apply(struct d_entity_world *w,const struct d_entity_command *c,size_t n){(void)w;(void)c;(void)n;return D_FAILED;}
int d_entity_query(const struct d_entity_world *w,unsigned r,unsigned e,struct d_entity_query *out){(void)w;(void)r;(void)e;(void)out;return D_FAILED;}
