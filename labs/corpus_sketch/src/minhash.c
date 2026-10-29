#include "sketch.h"
/* Learner task: implement actual permutation minima and banded candidates. */
int sk_mh_init(struct sk_mh *s,const struct sk_mh_config *c) { (void)s;(void)c;return SK_INVALID; }
int sk_mh_add(struct sk_mh *s,unsigned e) { (void)s;(void)e;return SK_INVALID; }
int sk_mh_similarity(const struct sk_mh *s,const struct sk_mh *t,struct sk_fraction *f) { (void)s;(void)t;(void)f;return SK_INVALID; }
int sk_mh_merge(struct sk_mh *s,const struct sk_mh *t) { (void)s;(void)t;return SK_INVALID; }
int sk_mh_bands(const struct sk_mh *s,const struct sk_mh *t,unsigned r,unsigned *n) { (void)s;(void)t;(void)r;(void)n;return SK_INVALID; }
