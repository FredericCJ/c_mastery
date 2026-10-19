#include "recovery.h"
uint32_t du_crc(const unsigned char*p,size_t n){(void)p;(void)n;return 0;}
int du_encode(const DuImage*i,const unsigned char*p,size_t n,DuSlot*o){(void)i;(void)p;(void)n;(void)o;return DU_UNIMPLEMENTED;}
int du_decode(const DuSlot*s,DuImage*o){(void)s;(void)o;return DU_UNIMPLEMENTED;}
int du_select(const DuSlot*s,size_t n,uint32_t f,uint32_t m,uint32_t c,unsigned*o){(void)s;(void)n;(void)f;(void)m;(void)c;(void)o;return DU_UNIMPLEMENTED;}
int du_install(DuSlot s[2],unsigned a,const unsigned e[2],const DuSlot*n,unsigned c,uint32_t m){(void)s;(void)a;(void)e;(void)n;(void)c;(void)m;return DU_UNIMPLEMENTED;}
int du_prepare(const DuImage*i,const DuMachine*m,DuPlan*o){(void)i;(void)m;(void)o;return DU_UNIMPLEMENTED;}
int du_retry_initialize(DuRetry*r){(void)r;return DU_UNIMPLEMENTED;}
int du_retry_state(const DuRetry*r,uint32_t l,uint32_t*o){(void)r;(void)l;(void)o;return DU_UNIMPLEMENTED;}
int du_retry_take(DuRetry*r,uint32_t l,unsigned c,uint32_t*o){(void)r;(void)l;(void)c;(void)o;return DU_UNIMPLEMENTED;}
int du_effect(DuRecovery*r,unsigned v){(void)r;(void)v;return DU_UNIMPLEMENTED;}
int du_fault(DuRecovery*r,uint32_t x,uint32_t s,int(*f)(void*,uint32_t),void*c){(void)r;(void)x;(void)s;(void)f;(void)c;return DU_UNIMPLEMENTED;}
int du_contain(DuRecovery*r,uint32_t s,int(*f)(void*),void*c){(void)r;(void)s;(void)f;(void)c;return DU_UNIMPLEMENTED;}
int du_recovery_input(DuRecovery*r,const unsigned char*p,size_t n,size_t z,unsigned t,uint32_t s){(void)r;(void)p;(void)n;(void)z;(void)t;(void)s;return DU_UNIMPLEMENTED;}
