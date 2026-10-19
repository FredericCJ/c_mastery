#include "effect_follow.h"
int ch_create(COAllocator a,const CHHandler*b,unsigned c,const unsigned char d[4],CHEffect**e){(void)a;(void)b;(void)c;(void)d;(void)e;return CO_UNIMPLEMENTED;}
int ch_step(CHEffect*a,const CHHandler*b,uint32_t*c){(void)a;(void)b;(void)c;return CO_UNIMPLEMENTED;}
int ch_clone(const CHEffect*a,CHEffect**b){(void)a;(void)b;return CO_UNIMPLEMENTED;}
int ch_abort(CHEffect*a){(void)a;return CO_UNIMPLEMENTED;}
int ch_destroy(CHEffect**a){(void)a;return CO_UNIMPLEMENTED;}
