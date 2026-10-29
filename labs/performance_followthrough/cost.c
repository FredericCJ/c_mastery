#include "cost.h"
/* Implement the contracts in cost.h; deliberate learner stubs. */
int pf_sort(struct pf_bytes*a,size_t n,unsigned b,struct pf_work*w){(void)a;(void)n;(void)b;(void)w;return 1;}
int pf_cache(struct pf_cache*c,unsigned i,unsigned w){(void)c;(void)i;(void)w;return 1;}
int pf_flush(struct pf_cache*c){(void)c;return 1;}
int pf_interval(uint64_t s,unsigned n,uint64_t e,unsigned m,uint64_t*o){(void)s;(void)n;(void)e;(void)m;(void)o;return 1;}
int pf_scale(struct pf_event a,struct pf_ratio*o){(void)a;(void)o;return 1;}
int pf_cpi(struct pf_event a,struct pf_event b,struct pf_ratio*o){(void)a;(void)b;(void)o;return 1;}
int pf_profile(const struct pf_hook*e,size_t n,unsigned d,struct pf_profile*o){(void)e;(void)n;(void)d;(void)o;return 1;}
int pf_cycle(struct pf_cycle c,struct pf_cycle_result*o){(void)c;(void)o;return PF_CYCLE_UNKNOWN;}
