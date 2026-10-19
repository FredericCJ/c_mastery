#include "balance.h"
int sb_init(SbWorld *w,unsigned cpus,unsigned capacity,SwPort port){(void)w;(void)cpus;(void)capacity;(void)port;return SQ_INVALID;}
int sb_create(SbWorld *w,unsigned actor,unsigned home,unsigned slot,unsigned mask,SqHandle *id,SqEvent *e){(void)w;(void)actor;(void)home;(void)slot;(void)mask;(void)id;(void)e;return SQ_INVALID;}
int sb_affinity(SbWorld *w,unsigned actor,SqHandle id,unsigned mask,SlStats *s){(void)w;(void)actor;(void)id;(void)mask;(void)s;return SQ_INVALID;}
int sb_dispatch(SbWorld *w,unsigned cpu,SqHandle *id,SqEvent *e,SlStats *s){(void)w;(void)cpu;(void)id;(void)e;(void)s;return SQ_INVALID;}
int sb_finish(SbWorld *w,unsigned cpu,SqHandle id,unsigned next,SqEvent *e,SlStats *s){(void)w;(void)cpu;(void)id;(void)next;(void)e;(void)s;return SQ_INVALID;}
int sb_wake(SbWorld *w,unsigned actor,SqHandle id,SqEvent *e,SlStats *s){(void)w;(void)actor;(void)id;(void)e;(void)s;return SQ_INVALID;}
int sb_move_begin(SbWorld *w,unsigned actor,unsigned dst,SqHandle id,unsigned fail,SqTransfer *ticket,SqEvent *e,SlStats *s){(void)w;(void)actor;(void)dst;(void)id;(void)fail;(void)ticket;(void)e;(void)s;return SQ_INVALID;}
int sb_move_end(SbWorld *w,unsigned actor,SqTransfer ticket,int commit,SqEvent *e,SlStats *s){(void)w;(void)actor;(void)ticket;(void)commit;(void)e;(void)s;return SQ_INVALID;}
int sb_balance(SbWorld *w,unsigned actor,SqEvent *begin,SqEvent *end,SlStats *s){(void)w;(void)actor;(void)begin;(void)end;(void)s;return SQ_INVALID;}
int sb_timer_arm(SbWorld *w,unsigned actor,SqHandle id,SbTimer *token,SlStats *s){(void)w;(void)actor;(void)id;(void)token;(void)s;return SQ_INVALID;}
int sb_timer_fire(SbWorld *w,unsigned actor,SbTimer token,SqEvent *e,SlStats *s){(void)w;(void)actor;(void)token;(void)e;(void)s;return SQ_INVALID;}
int sb_timer_cancel(SbWorld *w,unsigned actor,SbTimer token,SlStats *s){(void)w;(void)actor;(void)token;(void)s;return SQ_INVALID;}
int sb_retry(SbWorld *w,unsigned actor,unsigned dst,SlStats *s){(void)w;(void)actor;(void)dst;(void)s;return SQ_INVALID;}
int sb_prepare_idle(SbWorld *w,unsigned cpu,int masked,SlStats *s){(void)w;(void)cpu;(void)masked;(void)s;return SQ_INVALID;}
int sb_invariant(const SbWorld *w){(void)w;return 0;}
