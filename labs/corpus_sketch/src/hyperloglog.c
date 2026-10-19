#include "sketch.h"
/* Learner task: implement rank registers, estimator and precision transfer. */
int sk_hll_init(SkHll *s,unsigned p,uint32_t id) { (void)s;(void)p;(void)id;return SK_INVALID; }
int sk_hll_add(SkHll *s,uint32_t h) { (void)s;(void)h;return SK_INVALID; }
int sk_hll_estimate(const SkHll *s,double *v) { (void)s;(void)v;return SK_INVALID; }
int sk_hll_merge(SkHll *s,const SkHll *t) { (void)s;(void)t;return SK_INVALID; }
int sk_hll_fold(SkHll *s,const SkHll *t) { (void)s;(void)t;return SK_INVALID; }
