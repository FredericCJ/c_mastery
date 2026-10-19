#include "sync.h"
int su_configure(Su *s,PuIo i,unsigned p,unsigned h,unsigned l) {
  (void)s;(void)i;(void)p;(void)h;(void)l;return PU_UNIMPLEMENTED;
}
int su_enable(Su *s,PuIo i,unsigned t,unsigned r) {
  (void)s;(void)i;(void)t;(void)r;return PU_UNIMPLEMENTED;
}
int su_begin(Su *s,PuIo i,uint32_t g,uint32_t t,const uint8_t *d,unsigned n,
             PuByte *r,unsigned c,uint32_t now,uint32_t b) {
  (void)s;(void)i;(void)g;(void)t;(void)d;(void)n;(void)r;(void)c;(void)now;(void)b;
  return PU_UNIMPLEMENTED;
}
int su_step(Su *s,PuIo i,uint32_t t,uint32_t n) {
  (void)s;(void)i;(void)t;(void)n;return PU_UNIMPLEMENTED;
}
int su_cancel(Su *s,PuIo i,uint32_t t) {
  (void)s;(void)i;(void)t;return PU_UNIMPLEMENTED;
}
int su_trace(unsigned p,unsigned h,unsigned l,const SDSync *t,unsigned *b) {
  (void)p;(void)h;(void)l;(void)t;(void)b;return PU_UNIMPLEMENTED;
}
void su_f439_ck(void) { }
