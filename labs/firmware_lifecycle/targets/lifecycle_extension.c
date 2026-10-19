#include "lifecycle_extension.h"
int lce_admit(const unsigned char *p,size_t n,const unsigned char *s,size_t z,unsigned t,unsigned r,uint32_t f,LCEVerify v,void *c,uint32_t *o){(void)p;(void)n;(void)s;(void)z;(void)t;(void)r;(void)f;(void)v;(void)c;(void)o;return 0;}
int lce_record(unsigned char *p,uint32_t s,uint32_t v){(void)p;(void)s;(void)v;return 0;}
int lce_valid(const unsigned char *p,uint32_t *s,uint32_t *v){(void)p;(void)s;(void)v;return 0;}
int lce_choose(const unsigned char *a,const unsigned char *b,unsigned *o){(void)a;(void)b;(void)o;return 0;}
int lce_next(uint32_t c,uint32_t m,uint32_t *o){(void)c;(void)m;(void)o;return 0;}
int lce_joint_reset(LCEJoint *s){(void)s;return 0;}
int lce_joint_ack(LCEJoint *s,unsigned c,uint32_t g,unsigned p){(void)s;(void)c;(void)g;(void)p;return 0;}
int lce_joint_confirm(const LCEJoint *s){(void)s;return 0;}
int lce_fault_begin(LCEFault *s){(void)s;return 0;}
int lce_fault_commit(LCEFault *s,uint32_t p,uint32_t t){(void)s;(void)p;(void)t;return 0;}
