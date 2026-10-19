#include "scene.h"
void cs_init(CsScene *s) { (void)s; }
CsStatus cs_add(CsScene *s, const CsHandle *p, const CsMatrix *m,
                CsHandle *out) {
  (void)s;
  (void)p;
  (void)m;
  (void)out;
  return CS_INVALID;
}
CsStatus cs_set(CsScene *s, CsHandle h, const CsMatrix *m) {
  (void)s;
  (void)h;
  (void)m;
  return CS_INVALID;
}
CsStatus cs_world(CsScene *s, CsHandle h, CsMatrix *out, unsigned *w) {
  (void)s;
  (void)h;
  (void)out;
  (void)w;
  return CS_INVALID;
}
CsStatus cs_reparent(CsScene *s, CsHandle h, const CsHandle *p, unsigned k) {
  (void)s;
  (void)h;
  (void)p;
  (void)k;
  return CS_INVALID;
}
CsStatus cs_remove(CsScene *s, CsHandle h) {
  (void)s;
  (void)h;
  return CS_INVALID;
}
CsStatus nv_path(const NvPolygon *c, unsigned n, NvPoint a, NvPoint b,
                 NvScratch *s, unsigned p, unsigned q, NvPath *out) {
  (void)c;
  (void)n;
  (void)a;
  (void)b;
  (void)s;
  (void)p;
  (void)q;
  (void)out;
  return CS_INVALID;
}
void nv_init(NvSession *s) { (void)s; }
CsStatus nv_begin(NvSession *s, const NvPolygon *c, unsigned n, NvPoint a,
                  NvPoint b, NvScratch *w, unsigned p, unsigned q,
                  NvTicket *out) {
  (void)s;
  (void)c;
  (void)n;
  (void)a;
  (void)b;
  (void)w;
  (void)p;
  (void)q;
  (void)out;
  return CS_INVALID;
}
CsStatus nv_copy(const NvSession *s, NvTicket t, NvPath *out) {
  (void)s;
  (void)t;
  (void)out;
  return CS_INVALID;
}
CsStatus nv_cancel(NvSession *s, NvTicket t) {
  (void)s;
  (void)t;
  return CS_INVALID;
}
