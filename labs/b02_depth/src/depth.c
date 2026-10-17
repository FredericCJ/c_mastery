#include "depth.h"
/* Replace the compiling stubs. Public tests intentionally fail. */
int b2_parse(const unsigned char *p,size_t n,int32_t *o){(void)p;(void)n;(void)o;return B2_FAILED;}
void b2_map_init(B2Map *m){size_t i;for(i=0;i<7;i++)m->slots[i].state=0;m->count=0;}
int b2_map_put(B2Map *m,uint16_t k,int v){(void)m;(void)k;(void)v;return B2_FAILED;}
int b2_map_get(const B2Map *m,uint16_t k,int *v){(void)m;(void)k;(void)v;return B2_FAILED;}
int b2_map_remove(B2Map *m,uint16_t k){(void)m;(void)k;return B2_FAILED;}
int b2_sort(B2Item *a,size_t n,B2Item *s,size_t c){(void)a;(void)n;(void)s;(void)c;return B2_FAILED;}
void b2_arena_init(B2Arena *a){a->cursor=0;a->depth=0;a->next_id=1;}
int b2_reserve(B2Arena *a,size_t n,size_t l,size_t *o){(void)a;(void)n;(void)l;(void)o;return B2_FAILED;}
int b2_mark(B2Arena *a,B2Mark *m){(void)a;(void)m;return B2_FAILED;}
int b2_rollback(B2Arena *a,B2Mark m){(void)a;(void)m;return B2_FAILED;}
int b2_q7(const int8_t *a,const int8_t *b,size_t n,int8_t *o){(void)a;(void)b;(void)n;(void)o;return B2_FAILED;}
int b2_transaction(B2Backend b,unsigned *p){(void)b;(void)p;return B2_FAILED;}
int b2_render(const B2Result *r,unsigned char *o,size_t c,size_t *w){(void)r;(void)o;(void)c;(void)w;return B2_FAILED;}
int b2_identity_int(int x){return x+1;}
unsigned b2_identity_unsigned(unsigned x){return x+1;}
