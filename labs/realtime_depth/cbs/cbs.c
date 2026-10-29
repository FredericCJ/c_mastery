#include "cbs.h"
enum cmx_result cb_init(struct cb_budget *s,uint32_t q,uint32_t d,uint32_t p,enum cb_policy policy,uint32_t now) {
  (void)s;(void)q;(void)d;(void)p;(void)policy;(void)now;return CMX_UNIMPLEMENTED;
}
enum cmx_result cb_arrive(struct cb_budget *s,uint32_t now){(void)s;(void)now;return CMX_UNIMPLEMENTED;}
enum cmx_result cb_idle(struct cb_budget *s){(void)s;return CMX_UNIMPLEMENTED;}
enum cmx_result cb_update(struct cb_budget *s,uint32_t now){(void)s;(void)now;return CMX_UNIMPLEMENTED;}
enum cmx_result cb_charge(struct cb_budget *s,uint32_t now,uint32_t amount){(void)s;(void)now;(void)amount;return CMX_UNIMPLEMENTED;}
int cb_invariant(const struct cb_budget *s){(void)s;return 0;}
enum cmx_result cb_admit(uint32_t q0,uint32_t d0,uint32_t p0,uint32_t q1,uint32_t d1,uint32_t p1,unsigned *out){
  (void)q0;(void)d0;(void)p0;(void)q1;(void)d1;(void)p1;(void)out;return CMX_UNIMPLEMENTED;
}
enum cmx_result cb_workload(const struct cb_workload *w,struct cb_trace *t){(void)w;(void)t;return CMX_UNIMPLEMENTED;}
enum cmx_result cb_mutex(unsigned n,struct cb_mutex_trace *t){(void)n;(void)t;return CMX_UNIMPLEMENTED;}
