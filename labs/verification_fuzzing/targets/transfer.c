#include "transfer.h"
int vt_mul(uint32_t a,uint32_t b,uint32_t*out){(void)a;(void)b;(void)out;return 0;}
int vt_narrow(int64_t a,uint16_t*out){(void)a;(void)out;return 0;}
size_t vt_desc(const VTRecord*a,size_t n,int32_t k){(void)a;(void)n;(void)k;return 0;}
int vt_offer(VTQueue*q,const void*p,VTPrepare f,VTNotify g,void*c){(void)q;(void)p;(void)f;(void)g;(void)c;return 0;}
int vt_remove(VTQueue*q){(void)q;return 0;}
void vt_reset(VTStream*s){(void)s;}
int vt_feed(VTStream*s,const unsigned char*p,size_t n){(void)s;(void)p;(void)n;return 0;}
int vt_finish(const VTStream*s,uint32_t*out){(void)s;(void)out;return 0;}
unsigned vt_violation(const VTEvent*a,size_t n){(void)a;(void)n;return 0;}
size_t vt_shrink(VTEvent*a,size_t n){(void)a;return n;}
