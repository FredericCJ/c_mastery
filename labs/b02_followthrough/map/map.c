#include "map.h"
int bf_map_rehash(struct b2_map *m,struct b2_entry *s,size_t n,unsigned f){(void)m;(void)s;(void)n;(void)f;return B2_FAILED;}
int bf_borrow_put(struct bf_borrow_map*m,const unsigned char*p,size_t n,int v){(void)m;(void)p;(void)n;(void)v;return B2_FAILED;}
int bf_borrow_get(const struct bf_borrow_map*m,const unsigned char*p,size_t n,int*out){(void)m;(void)p;(void)n;(void)out;return B2_FAILED;}
