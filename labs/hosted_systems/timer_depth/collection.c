/* Learner scaffold. Derive the collection invariants before filling these in. */
#include "collection.h"
int tc_init(TcCollection *q, enum TcKind k, TdTimer *t, size_t c, size_t *i,
            size_t *p, size_t *n, size_t *v, size_t *h, size_t b, uint64_t u, unsigned g) {
  (void)q;(void)k;(void)t;(void)c;(void)i;(void)p;(void)n;(void)v;(void)h;(void)b;(void)u;(void)g;
  return -1;
}
int tc_arm(TcCollection *q,size_t s,uint64_t d,uint64_t p,int k,TcToken *t) {
  (void)q;(void)s;(void)d;(void)p;(void)k;(void)t;return -1;
}
int tc_cancel(TcCollection *q,TcToken t) {(void)q;(void)t;return -1;}
int tc_peek(TcCollection *q,TcToken *t,uint64_t *d) {(void)q;(void)t;(void)d;return -1;}
int tc_begin(TcCollection *q,uint64_t n,TcToken *t,TdDispatch *d) {(void)q;(void)n;(void)t;(void)d;return -1;}
int tc_finish(TcCollection *q,TcToken t) {(void)q;(void)t;return -1;}
