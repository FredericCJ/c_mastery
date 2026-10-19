#include "dod_entities.h"
void d_entity_init(DEntityWorld *w){(void)w;}
int d_entity_create(DEntityWorld *w,unsigned m,int p,int v,DHandle *out){(void)w;(void)m;(void)p;(void)v;(void)out;return D_FAILED;}
int d_entity_apply(DEntityWorld *w,const DEntityCommand *c,size_t n){(void)w;(void)c;(void)n;return D_FAILED;}
int d_entity_query(const DEntityWorld *w,unsigned r,unsigned e,DEntityQuery *out){(void)w;(void)r;(void)e;(void)out;return D_FAILED;}
