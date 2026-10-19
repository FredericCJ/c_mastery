#include "cost.h"
/* Implement the contracts in cost.h; deliberate learner stubs. */
int pf_sort(PFBytes*a,size_t n,unsigned b,PFWork*w){(void)a;(void)n;(void)b;(void)w;return 1;}
int pf_cache(PFCache*c,unsigned i,unsigned w){(void)c;(void)i;(void)w;return 1;}
int pf_flush(PFCache*c){(void)c;return 1;}
int pf_interval(uint64_t s,unsigned n,uint64_t e,unsigned m,uint64_t*o){(void)s;(void)n;(void)e;(void)m;(void)o;return 1;}
int pf_scale(PFEvent a,PFRatio*o){(void)a;(void)o;return 1;}
int pf_cpi(PFEvent a,PFEvent b,PFRatio*o){(void)a;(void)b;(void)o;return 1;}
int pf_profile(const PFHook*e,size_t n,unsigned d,PFProfile*o){(void)e;(void)n;(void)d;(void)o;return 1;}
int pf_cycle(PFCycle c,PFCycleResult*o){(void)c;(void)o;return PF_CYCLE_UNKNOWN;}
