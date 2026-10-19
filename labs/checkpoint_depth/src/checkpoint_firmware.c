#include "checkpoint_firmware.h"
int cpf_ring_init(CPFRing *r,unsigned n){(void)r;(void)n;return 0;}
int cpf_irq_push(CPFRing *r,CPFSample s){(void)r;(void)s;return 0;}
int cpf_take(CPFRing *r,CPFSample *s){(void)r;(void)s;return 0;}
CPStatus cpf_frame(const CPFSample *s,unsigned char *b,size_t n,size_t *w){(void)s;(void)b;(void)n;(void)w;return CP_INVALID;}
void cpf_tx_init(CPFTx *t,unsigned l,unsigned r){(void)t;(void)l;(void)r;}
CPFStatus cpf_tx_begin(CPFTx *t,const CPMessage *m,int d){(void)t;(void)m;(void)d;return CPF_INVALID;}
CPFStatus cpf_tx_poll(CPFTx *t,unsigned g,int e,int c,CPFWrite w,void *x){(void)t;(void)g;(void)e;(void)c;(void)w;(void)x;return CPF_INVALID;}
CPFStatus cpf_tx_dma(CPFTx *t,unsigned g,int e,size_t n,int c,int f){(void)t;(void)g;(void)e;(void)n;(void)c;(void)f;return CPF_INVALID;}
CPFStatus cpf_tx_timeout(CPFTx *t,unsigned g,uint32_t c){(void)t;(void)g;(void)c;return CPF_INVALID;}
CPFStatus cpf_tx_recover(CPFTx *t,unsigned g,int d,int u){(void)t;(void)g;(void)d;(void)u;return CPF_INVALID;}
CPFStatus cpf_tx_next(CPFTx *t){(void)t;return CPF_INVALID;}
int cpf_clock_init(CPFClock *c){(void)c;return 0;}
int cpf_clock_write(CPFClock *c,uint64_t v){(void)c;(void)v;return 0;}
int cpf_clock_read(const CPFClock *c,unsigned n,uint64_t *v){(void)c;(void)n;(void)v;return 0;}
int cpf_elapsed16(uint16_t a,uint16_t b,uint16_t *v){(void)a;(void)b;(void)v;return 0;}
