#include "result.h"
int bf_number(int a,unsigned b,B2Result*c){(void)a;(void)b;(void)c;return B2_FAILED;}
int bf_text(const unsigned char*a,size_t b,B2Result*c){(void)a;(void)b;(void)c;return B2_FAILED;}
void bf_value_init(BfValue*v){v->tag=BF_EMPTY;}
int bf_clone(BfValue*a,const unsigned char*b,size_t c,BfAlloc d,BfFree e,void*f){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;return B2_FAILED;}
int bf_move(BfValue*a,BfValue*b){(void)a;(void)b;return B2_FAILED;}
int bf_take(BfValue*a,BfBytes*b){(void)a;(void)b;return B2_FAILED;}
void bf_bytes_drop(BfBytes*a){(void)a;}
void bf_reset(BfValue*a){(void)a;}
int bf_value_render(const BfValue*a,unsigned char*b,size_t c,size_t*d){(void)a;(void)b;(void)c;(void)d;return B2_FAILED;}
int bf_encode(const BfWire*a,unsigned char*b,size_t c,size_t*d){(void)a;(void)b;(void)c;(void)d;return B2_FAILED;}
int bf_decode(const unsigned char*a,size_t b,BfWire*c){(void)a;(void)b;(void)c;return B2_FAILED;}
