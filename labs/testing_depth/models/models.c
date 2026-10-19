/* Deliberately incomplete learner extension; imports completed architecture. */
#include "models.h"
void tm_init(TmQueue*q){(void)q;}
TaStatus tm_push(TmQueue*q,unsigned char v){(void)q;(void)v;return TA_INVALID;}
TaStatus tm_pop(TmQueue*q,unsigned char*v){(void)q;(void)v;return TA_INVALID;}
TaStatus tm_clear(TmQueue*q){(void)q;return TA_INVALID;}
TaStatus tm_borrow(const TmQueue*q,TmBorrow*b){(void)q;(void)b;return TA_INVALID;}
TaStatus tm_read(TmBorrow b,unsigned char*v){(void)b;(void)v;return TA_INVALID;}
unsigned tm_decode(const unsigned char*p,size_t n,TmPacket*o){(void)p;(void)n;(void)o;return TM_INVALID;}
unsigned tm_reference(const unsigned char*p,size_t n,int a,TmPacket*o){(void)p;(void)n;(void)a;(void)o;return TM_INVALID;}
unsigned tm_fir(TmFir*f,const int32_t*p,size_t n,int s,int64_t*o){(void)f;(void)p;(void)n;(void)s;(void)o;return TM_INVALID;}
