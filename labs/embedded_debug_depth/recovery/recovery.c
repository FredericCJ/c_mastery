#include "recovery.h"
uint32_t du_crc(const unsigned char*p,size_t n){(void)p;(void)n;return 0;}
int du_encode(const struct du_image*i,const unsigned char*p,size_t n,struct du_slot*o){(void)i;(void)p;(void)n;(void)o;return DU_UNIMPLEMENTED;}
int du_decode(const struct du_slot*s,struct du_image*o){(void)s;(void)o;return DU_UNIMPLEMENTED;}
int du_select(const struct du_slot*s,size_t n,uint32_t f,uint32_t m,uint32_t c,unsigned*o){(void)s;(void)n;(void)f;(void)m;(void)c;(void)o;return DU_UNIMPLEMENTED;}
int du_install(struct du_slot s[2],unsigned a,const unsigned e[2],const struct du_slot*n,unsigned c,uint32_t m){(void)s;(void)a;(void)e;(void)n;(void)c;(void)m;return DU_UNIMPLEMENTED;}
int du_prepare(const struct du_image*i,const struct du_machine*m,struct du_plan*o){(void)i;(void)m;(void)o;return DU_UNIMPLEMENTED;}
int du_retry_initialize(struct du_retry*r){(void)r;return DU_UNIMPLEMENTED;}
int du_retry_state(const struct du_retry*r,uint32_t l,uint32_t*o){(void)r;(void)l;(void)o;return DU_UNIMPLEMENTED;}
int du_retry_take(struct du_retry*r,uint32_t l,unsigned c,uint32_t*o){(void)r;(void)l;(void)c;(void)o;return DU_UNIMPLEMENTED;}
int du_effect(struct du_recovery*r,unsigned v){(void)r;(void)v;return DU_UNIMPLEMENTED;}
int du_fault(struct du_recovery*r,uint32_t x,uint32_t s,int(*f)(void*,uint32_t),void*c){(void)r;(void)x;(void)s;(void)f;(void)c;return DU_UNIMPLEMENTED;}
int du_contain(struct du_recovery*r,uint32_t s,int(*f)(void*),void*c){(void)r;(void)s;(void)f;(void)c;return DU_UNIMPLEMENTED;}
int du_recovery_input(struct du_recovery*r,const unsigned char*p,size_t n,size_t z,unsigned t,uint32_t s){(void)r;(void)p;(void)n;(void)z;(void)t;(void)s;return DU_UNIMPLEMENTED;}
