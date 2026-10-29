#include "system_follow.h"
int pf_system_init(struct pf_system *s,struct fw_io io,uint32_t e,pf_free_fn f,void *c){(void)s;(void)io;(void)e;(void)f;(void)c;return PT_UNIMPLEMENTED;}
int pf_system_post(struct pf_system *s,unsigned p,unsigned k,struct pf_recovery r,uint32_t e,unsigned i,struct ad_brand *b){(void)s;(void)p;(void)k;(void)r;(void)e;(void)i;(void)b;return PT_UNIMPLEMENTED;}
int pf_system_step(struct pf_system *s){(void)s;return PT_UNIMPLEMENTED;}
int pf_system_arm_deadline(struct pf_system*s,struct pf_recovery r,uint64_t n,unsigned t){(void)s;(void)r;(void)n;(void)t;return PT_UNIMPLEMENTED;}
int pf_system_tick(struct pf_system*s,uint64_t n){(void)s;(void)n;return PT_UNIMPLEMENTED;}
