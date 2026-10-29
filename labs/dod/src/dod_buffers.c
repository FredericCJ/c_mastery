#include "dod_buffers.h"
int d_inline_append(struct d_inline *s,const unsigned char *p,size_t n){(void)s;(void)p;(void)n;return D_FAILED;}
int d_inline_truncate(struct d_inline *s,size_t n){(void)s;(void)n;return D_FAILED;}
void d_zero_init(struct d_zero *s,struct d_quota *q){*s=(struct d_zero){q,NULL,0};}
int d_zero_append(struct d_zero *s,const unsigned char *p,size_t n){(void)s;(void)p;(void)n;return D_FAILED;}
int d_zero_pair_append(struct d_zero *a,const unsigned char *p,size_t n,struct d_zero *b,const unsigned char *q,size_t m){(void)a;(void)p;(void)n;(void)b;(void)q;(void)m;return D_FAILED;}
int d_zero_move(struct d_zero *a,struct d_zero *b){(void)a;(void)b;return D_FAILED;}
void d_zero_reset(struct d_zero *s){(void)s;}
uint32_t d_swar16_signed(uint32_t a,uint32_t b){(void)a;(void)b;return 0;}
