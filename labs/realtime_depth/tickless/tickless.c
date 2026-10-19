#include "tickless.h"
enum tq_status tq_relation(uint32_t a,uint32_t b,unsigned c,unsigned d,enum tq_relation *e){(void)a;(void)b;(void)c;(void)d;(void)e;return TQ_UNIMPLEMENTED;}
enum tq_status tq_ticks(uint64_t a,uint32_t b,uint32_t c,uint64_t *d){(void)a;(void)b;(void)c;(void)d;return TQ_UNIMPLEMENTED;}
enum tq_status tq_not_before(uint64_t a,uint64_t b,uint32_t c,uint32_t d,uint64_t *e){(void)a;(void)b;(void)c;(void)d;(void)e;return TQ_UNIMPLEMENTED;}
enum tq_status tq_init(TQTimer *a,uint32_t b){(void)a;(void)b;return TQ_UNIMPLEMENTED;}
enum tq_status tq_wait(TQTimer *a,unsigned b,uint32_t c,unsigned d){(void)a;(void)b;(void)c;(void)d;return TQ_UNIMPLEMENTED;}
enum tq_status tq_cancel(TQTimer *a,unsigned b){(void)a;(void)b;return TQ_UNIMPLEMENTED;}
enum tq_status tq_advance(TQTimer *a,uint32_t b){(void)a;(void)b;return TQ_UNIMPLEMENTED;}
enum tq_status tq_collect(TQTimer *a,unsigned b,uint32_t *c,enum cmx_result *d){(void)a;(void)b;(void)c;(void)d;return TQ_UNIMPLEMENTED;}
enum tq_status tq_plan(TQTimer *a,uint32_t b,TQPlan *c){(void)a;(void)b;(void)c;return TQ_UNIMPLEMENTED;}
enum tq_status tq_commit(TQTimer *a,TQPlan b,uint32_t c,unsigned d){(void)a;(void)b;(void)c;(void)d;return TQ_UNIMPLEMENTED;}
enum tq_status tq_abort(TQTimer *a){(void)a;return TQ_UNIMPLEMENTED;}
enum tq_status tq_resume(TQTimer *a,TQResume *b,uint32_t c){(void)a;(void)b;(void)c;return TQ_UNIMPLEMENTED;}
