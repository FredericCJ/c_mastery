#include "b04.h"
int b4_serial(uint32_t a,uint32_t b,unsigned n,int *r){(void)a;(void)b;(void)n;(void)r;return B4_UNIMPLEMENTED;}
int b4_window(uint32_t a,uint32_t b,uint32_t n,unsigned bits,int *r){(void)a;(void)b;(void)n;(void)bits;(void)r;return B4_UNIMPLEMENTED;}
int b4_certificate(const struct b4_graph*g,unsigned s,const uint32_t*d,const unsigned*p){(void)g;(void)s;(void)d;(void)p;return B4_UNIMPLEMENTED;}
int b4_framing(const unsigned char*p,size_t n,size_t l,struct b4_framing*r){(void)p;(void)n;(void)l;(void)r;return B4_UNIMPLEMENTED;}
int b4_dma(struct b4_dma*d,unsigned e,uint32_t g){(void)d;(void)e;(void)g;return B4_UNIMPLEMENTED;}
int b4_cache_extent(size_t o,size_t n,size_t l,size_t c,size_t*b,size_t*e){(void)o;(void)n;(void)l;(void)c;(void)b;(void)e;return B4_UNIMPLEMENTED;}
int b4_memory(struct b4_ledger*l,unsigned o,uint32_t i,size_t b,int s){(void)l;(void)o;(void)i;(void)b;(void)s;return B4_UNIMPLEMENTED;}
