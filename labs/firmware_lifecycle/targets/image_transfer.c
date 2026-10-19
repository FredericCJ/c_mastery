#include "image_transfer.h"
int fi_init(FIImage*s,unsigned u){(void)s;(void)u;return 0;}
int fi_program(FIImage*s,const unsigned char*p,unsigned i,unsigned b,int c){(void)s;(void)p;(void)i;(void)b;(void)c;return 0;}
int fi_commit(const FIImage*s,const unsigned char*p,uint32_t n,unsigned char r[24]){(void)s;(void)p;(void)n;(void)r;return 0;}
int fi_record_matches(const unsigned char p[32],const unsigned char r[24]){(void)p;(void)r;return 0;}
int fi_advance(FIRotation*s){(void)s;return 0;}
int fi_rotate(FIRotation*s,uint32_t e,unsigned k,uint32_t f,unsigned v,int r,FIPersist p,void*c){(void)s;(void)e;(void)k;(void)f;(void)v;(void)r;(void)p;(void)c;return 0;}
