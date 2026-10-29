#include "halfduplex.h"
enum cpf_status cpx_halfduplex_begin(struct cpx_half_duplex *h,const struct cp_message *m,uint16_t d,cpx_direction_fn f,void *c){(void)h;(void)m;(void)d;(void)f;(void)c;return CPF_INVALID;}
enum cpf_status cpx_halfduplex_step(struct cpx_half_duplex *h,uint16_t n,int e,int t,cpf_write_fn w,void *c){(void)h;(void)n;(void)e;(void)t;(void)w;(void)c;return CPF_INVALID;}
