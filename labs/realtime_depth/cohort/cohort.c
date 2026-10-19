#include "cohort.h"
/* Unsolved learner scaffold. */
unsigned su_available(const SUState *s){(void)s;return 0;}
int su_invariant(const SUState *s){(void)s;return 0;}
enum cmx_result su_init(SUState *s,unsigned q,uint32_t p,uint32_t t,unsigned o){
 (void)s;(void)q;(void)p;(void)t;(void)o;return CMX_INVALID;
}
enum cmx_result su_active(SUState *s,uint32_t t,unsigned a){(void)s;(void)t;(void)a;return CMX_INVALID;}
enum cmx_result su_run(SUState *s,uint32_t t,unsigned n){(void)s;(void)t;(void)n;return CMX_INVALID;}
