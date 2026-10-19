#include "sketch.h"
/* Learner task: implement actual permutation minima and banded candidates. */
int sk_mh_init(SkMh *s,const SkMhConfig *c) { (void)s;(void)c;return SK_INVALID; }
int sk_mh_add(SkMh *s,unsigned e) { (void)s;(void)e;return SK_INVALID; }
int sk_mh_similarity(const SkMh *s,const SkMh *t,SkFraction *f) { (void)s;(void)t;(void)f;return SK_INVALID; }
int sk_mh_merge(SkMh *s,const SkMh *t) { (void)s;(void)t;return SK_INVALID; }
int sk_mh_bands(const SkMh *s,const SkMh *t,unsigned r,unsigned *n) { (void)s;(void)t;(void)r;(void)n;return SK_INVALID; }
