#include "timers.h"
int rzt_init(RztTimers *t,RztKind k,uint64_t n){(void)t;(void)k;(void)n;return RZ_INVALID;}
int rzt_schedule(RztTimers *t,uint64_t n,uint64_t d,RztId *o){(void)t;(void)n;(void)d;(void)o;return RZ_INVALID;}
int rzt_cancel(RztTimers *t,RztId i){(void)t;(void)i;return RZ_INVALID;}
int rzt_advance(RztTimers *t,uint64_t n,RztEvents *o){(void)t;(void)n;(void)o;return RZ_INVALID;}
int rzt_invariant(const RztTimers *t){(void)t;return 0;}
