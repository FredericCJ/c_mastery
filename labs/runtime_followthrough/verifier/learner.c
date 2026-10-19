#include "verifier.h"
int vv_verify(const CvmProgram *p, unsigned d, unsigned i, VvWork *w,
              VvReport *r) {
  (void)p;
  (void)d;
  (void)i;
  (void)w;
  (void)r;
  return VV_UNIMPLEMENTED;
}
int vv_load(const uint8_t *b, size_t n, unsigned d, unsigned i, VvWork *w,
            CvmProgram *p, VvReport *r) {
  (void)b;
  (void)n;
  (void)d;
  (void)i;
  (void)w;
  (void)p;
  (void)r;
  return VV_UNIMPLEMENTED;
}
int vv_lower(const CvmProgram *p, unsigned d, VvWork *w, CvmProgram *out,
             VvReport *r) {
  (void)p;
  (void)d;
  (void)w;
  (void)out;
  (void)r;
  return VV_UNIMPLEMENTED;
}
