#include "execution.h"
int vs_start(Vs *s, const CvmProgram *p, unsigned m, uint32_t f, uint32_t g,
             uint32_t h, uint32_t c, VvWork *w) {
  (void)s;
  (void)p;
  (void)m;
  (void)f;
  (void)g;
  (void)h;
  (void)c;
  (void)w;
  return VS_UNIMPLEMENTED;
}
int vs_step(Vs *s, CvmValue *v) {
  (void)s;
  (void)v;
  return VS_UNIMPLEMENTED;
}
int vs_take(Vs *s, VsRequest *r) {
  (void)s;
  (void)r;
  return VS_UNIMPLEMENTED;
}
int vs_complete(Vs *s, uint32_t e, uint32_t n, int f, int32_t v) {
  (void)s;
  (void)e;
  (void)n;
  (void)f;
  (void)v;
  return VS_UNIMPLEMENTED;
}
int vs_arithmetic(unsigned m, unsigned o, int32_t a, int32_t b, int32_t *v) {
  (void)m;
  (void)o;
  (void)a;
  (void)b;
  (void)v;
  return VS_UNIMPLEMENTED;
}
