#include "variants.h"
/* Deliberately unsolved: implement the named delayed variants. */
void sv_init(SVStream *s) { (void)s; }
int sv_start(SVStream *s, const unsigned char *b, size_t n, SVHandle *h) {
  (void)s;
  (void)b;
  (void)n;
  (void)h;
  return SF_UNIMPLEMENTED;
}
int sv_pump(SVHandle h, SVWrite w, void *c, unsigned b, size_t *n) {
  (void)h;
  (void)w;
  (void)c;
  (void)b;
  (void)n;
  return SF_UNIMPLEMENTED;
}
int sv_pumpv(SVHandle h, SVWriteV w, void *c, unsigned b, size_t *n) {
  (void)h;
  (void)w;
  (void)c;
  (void)b;
  (void)n;
  return SF_UNIMPLEMENTED;
}
int sv_cancel(SVHandle h) {
  (void)h;
  return SF_UNIMPLEMENTED;
}
void svq_init(SVQueue *q) { (void)q; }
int svq_submit(SVQueue *q, const unsigned char *b, size_t n, SVHandle *h) {
  (void)q;
  (void)b;
  (void)n;
  (void)h;
  return SF_UNIMPLEMENTED;
}
int svq_pump(SVQueue *q, SVWrite w, void *c, unsigned b, SVEvent *e) {
  (void)q;
  (void)w;
  (void)c;
  (void)b;
  (void)e;
  return SF_UNIMPLEMENTED;
}
int svq_cancel(SVQueue *q, SVHandle h, SVEvent *e) {
  (void)q;
  (void)h;
  (void)e;
  return SF_UNIMPLEMENTED;
}
int svq_reset_transport(SVQueue *q, unsigned v) {
  (void)q;
  (void)v;
  return SF_UNIMPLEMENTED;
}
