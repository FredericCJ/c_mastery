/* Intentionally unfinished learner implementation. Do not ship assessor source. */
#include "architecture.h"
TaStatus ta_init(TaQueue*q,TaStorage*s,TaLayout l,unsigned c,unsigned e){(void)q;(void)s;(void)l;(void)c;(void)e;return TA_INVALID;}
TaStatus ta_push(TaQueue*q,unsigned char v){(void)q;(void)v;return TA_INVALID;}
TaStatus ta_pop(TaQueue*q,unsigned char*v){(void)q;(void)v;return TA_INVALID;}
TaStatus ta_peek(const TaQueue*q,unsigned char*v){(void)q;(void)v;return TA_INVALID;}
size_t ta_count(const TaQueue*q){(void)q;return 0;}
TaStatus ta_batch(TaQueue*q,const unsigned char*p,size_t n,size_t*a){(void)q;(void)p;(void)n;(void)a;return TA_INVALID;}
int ta_invariant(const TaQueue*q){(void)q;return 0;}
TaStatus ta_decode(const unsigned char*p,size_t n,unsigned char o[2]){(void)p;(void)n;(void)o;return TA_INVALID;}
TaStatus ta_begin(TaFlow*f,const unsigned char*p,size_t n,TaLayout l,unsigned b){(void)f;(void)p;(void)n;(void)l;(void)b;return TA_INVALID;}
TaStatus ta_step(TaFlow*f,TaWrite w,void*c){(void)f;(void)w;(void)c;return TA_INVALID;}
TaStatus ta_cancel(TaFlow*f){(void)f;return TA_INVALID;}
