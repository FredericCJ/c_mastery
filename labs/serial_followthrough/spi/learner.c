#include "scheduler.h"
int ss_open(struct ss_scheduler *a,struct ps_io b,const struct pu_clock *c,unsigned d,unsigned e){(void)a;(void)b;(void)c;(void)d;(void)e;return PS_UNIMPLEMENTED;}
int ss_enqueue(struct ss_scheduler *a,uint32_t b,const uint8_t *c,unsigned d,unsigned e,unsigned f,unsigned g,unsigned h){(void)a;(void)b;(void)c;(void)d;(void)e;(void)f;(void)g;(void)h;return PS_UNIMPLEMENTED;}
int ss_run(struct ss_scheduler *a,struct ps_io b,ss_submit_fn c,uint32_t d){(void)a;(void)b;(void)c;(void)d;return PS_UNIMPLEMENTED;}
int ss_event(struct ss_scheduler *a,struct ps_io b,uint32_t c,uint32_t d){(void)a;(void)b;(void)c;(void)d;return PS_UNIMPLEMENTED;}
int ss_cancel(struct ss_scheduler *a,struct ps_io b,uint32_t c){(void)a;(void)b;(void)c;return PS_UNIMPLEMENTED;}
int ss_reset(struct ss_scheduler *a,struct ps_io b,uint32_t c,uint32_t d){(void)a;(void)b;(void)c;(void)d;return PS_UNIMPLEMENTED;}
int ss_take(struct ss_scheduler *a,uint32_t b,struct ss_result *c,uint8_t d[16]){(void)a;(void)b;(void)c;(void)d;return PS_UNIMPLEMENTED;}
