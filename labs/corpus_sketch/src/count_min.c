#include "sketch.h"
/* Learner task: implement weighted counting and transactional partition merge. */
int sk_cm_init(SkCm *s,const SkCmConfig *c) { (void)s;(void)c;return SK_INVALID; }
int sk_cm_add(SkCm *s,uint32_t k,uint64_t n) { (void)s;(void)k;(void)n;return SK_INVALID; }
int sk_cm_query(const SkCm *s,uint32_t k,uint64_t *n) { (void)s;(void)k;(void)n;return SK_INVALID; }
int sk_cm_merge(SkCm *s,const SkCm *t) { (void)s;(void)t;return SK_INVALID; }
