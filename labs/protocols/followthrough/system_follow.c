#include "system_follow.h"
int pf_system_init(PFSystem *s,FwIo io,uint32_t e,PFFree f,void *c){(void)s;(void)io;(void)e;(void)f;(void)c;return PT_UNIMPLEMENTED;}
int pf_system_post(PFSystem *s,unsigned p,unsigned k,PFRecovery r,uint32_t e,unsigned i,ADBrand *b){(void)s;(void)p;(void)k;(void)r;(void)e;(void)i;(void)b;return PT_UNIMPLEMENTED;}
int pf_system_step(PFSystem *s){(void)s;return PT_UNIMPLEMENTED;}
int pf_system_arm_deadline(PFSystem*s,PFRecovery r,uint64_t n,unsigned t){(void)s;(void)r;(void)n;(void)t;return PT_UNIMPLEMENTED;}
int pf_system_tick(PFSystem*s,uint64_t n){(void)s;(void)n;return PT_UNIMPLEMENTED;}
