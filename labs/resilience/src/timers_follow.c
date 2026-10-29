#include "timers.h"
int rzt_init(struct rzt_timers *t,enum rzt_kind k,uint64_t n){(void)t;(void)k;(void)n;return RZ_INVALID;}
int rzt_schedule(struct rzt_timers *t,uint64_t n,uint64_t d,struct rzt_id *o){(void)t;(void)n;(void)d;(void)o;return RZ_INVALID;}
int rzt_cancel(struct rzt_timers *t,struct rzt_id i){(void)t;(void)i;return RZ_INVALID;}
int rzt_advance(struct rzt_timers *t,uint64_t n,struct rzt_events *o){(void)t;(void)n;(void)o;return RZ_INVALID;}
int rzt_invariant(const struct rzt_timers *t){(void)t;return 0;}
