#include "cohort.h"
/* Unsolved learner scaffold. */
unsigned su_available(const struct su_state *s){(void)s;return 0;}
int su_invariant(const struct su_state *s){(void)s;return 0;}
enum cmx_result su_init(struct su_state *s,unsigned q,uint32_t p,uint32_t t,unsigned o){
 (void)s;(void)q;(void)p;(void)t;(void)o;return CMX_INVALID;
}
enum cmx_result su_active(struct su_state *s,uint32_t t,unsigned a){(void)s;(void)t;(void)a;return CMX_INVALID;}
enum cmx_result su_run(struct su_state *s,uint32_t t,unsigned n){(void)s;(void)t;(void)n;return CMX_INVALID;}
