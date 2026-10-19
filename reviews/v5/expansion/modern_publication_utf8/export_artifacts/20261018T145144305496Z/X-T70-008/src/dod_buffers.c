#include "dod_buffers.h"
int d_inline_append(DInline *s,const unsigned char *p,size_t n){(void)s;(void)p;(void)n;return D_FAILED;}
int d_inline_truncate(DInline *s,size_t n){(void)s;(void)n;return D_FAILED;}
void d_zero_init(DZero *s,DQuota *q){*s=(DZero){q,NULL,0};}
int d_zero_append(DZero *s,const unsigned char *p,size_t n){(void)s;(void)p;(void)n;return D_FAILED;}
int d_zero_pair_append(DZero *a,const unsigned char *p,size_t n,DZero *b,const unsigned char *q,size_t m){(void)a;(void)p;(void)n;(void)b;(void)q;(void)m;return D_FAILED;}
int d_zero_move(DZero *a,DZero *b){(void)a;(void)b;return D_FAILED;}
void d_zero_reset(DZero *s){(void)s;}
uint32_t d_swar16_signed(uint32_t a,uint32_t b){(void)a;(void)b;return 0;}
