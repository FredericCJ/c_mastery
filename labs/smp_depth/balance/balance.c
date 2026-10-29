#include "balance.h"
int sb_init(struct sb_world *w,unsigned cpus,unsigned capacity,struct sw_port port){(void)w;(void)cpus;(void)capacity;(void)port;return SQ_INVALID;}
int sb_create(struct sb_world *w,unsigned actor,unsigned home,unsigned slot,unsigned mask,struct sq_handle *id,struct sq_event *e){(void)w;(void)actor;(void)home;(void)slot;(void)mask;(void)id;(void)e;return SQ_INVALID;}
int sb_affinity(struct sb_world *w,unsigned actor,struct sq_handle id,unsigned mask,struct sl_stats *s){(void)w;(void)actor;(void)id;(void)mask;(void)s;return SQ_INVALID;}
int sb_dispatch(struct sb_world *w,unsigned cpu,struct sq_handle *id,struct sq_event *e,struct sl_stats *s){(void)w;(void)cpu;(void)id;(void)e;(void)s;return SQ_INVALID;}
int sb_finish(struct sb_world *w,unsigned cpu,struct sq_handle id,unsigned next,struct sq_event *e,struct sl_stats *s){(void)w;(void)cpu;(void)id;(void)next;(void)e;(void)s;return SQ_INVALID;}
int sb_wake(struct sb_world *w,unsigned actor,struct sq_handle id,struct sq_event *e,struct sl_stats *s){(void)w;(void)actor;(void)id;(void)e;(void)s;return SQ_INVALID;}
int sb_move_begin(struct sb_world *w,unsigned actor,unsigned dst,struct sq_handle id,unsigned fail,struct sq_transfer *ticket,struct sq_event *e,struct sl_stats *s){(void)w;(void)actor;(void)dst;(void)id;(void)fail;(void)ticket;(void)e;(void)s;return SQ_INVALID;}
int sb_move_end(struct sb_world *w,unsigned actor,struct sq_transfer ticket,int commit,struct sq_event *e,struct sl_stats *s){(void)w;(void)actor;(void)ticket;(void)commit;(void)e;(void)s;return SQ_INVALID;}
int sb_balance(struct sb_world *w,unsigned actor,struct sq_event *begin,struct sq_event *end,struct sl_stats *s){(void)w;(void)actor;(void)begin;(void)end;(void)s;return SQ_INVALID;}
int sb_timer_arm(struct sb_world *w,unsigned actor,struct sq_handle id,struct sb_timer *token,struct sl_stats *s){(void)w;(void)actor;(void)id;(void)token;(void)s;return SQ_INVALID;}
int sb_timer_fire(struct sb_world *w,unsigned actor,struct sb_timer token,struct sq_event *e,struct sl_stats *s){(void)w;(void)actor;(void)token;(void)e;(void)s;return SQ_INVALID;}
int sb_timer_cancel(struct sb_world *w,unsigned actor,struct sb_timer token,struct sl_stats *s){(void)w;(void)actor;(void)token;(void)s;return SQ_INVALID;}
int sb_retry(struct sb_world *w,unsigned actor,unsigned dst,struct sl_stats *s){(void)w;(void)actor;(void)dst;(void)s;return SQ_INVALID;}
int sb_prepare_idle(struct sb_world *w,unsigned cpu,int masked,struct sl_stats *s){(void)w;(void)cpu;(void)masked;(void)s;return SQ_INVALID;}
int sb_invariant(const struct sb_world *w){(void)w;return 0;}
