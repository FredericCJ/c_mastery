#include "diagnostic.h"
/* Learner scaffold: implement the contracts without an assessor fallback. */
int dg_encode(const DGMessage *m, uint8_t *p, size_t n, size_t *w) {
  (void)m;
  (void)p;
  (void)n;
  (void)w;
  return DG_UNIMPLEMENTED;
}
int dg_decode(const uint8_t *p, size_t n, DGMessage *m) {
  (void)p;
  (void)n;
  (void)m;
  return DG_UNIMPLEMENTED;
}
int dg_accept(DGSession *s, const uint8_t *p, size_t n, DGMessage *m) {
  (void)s;
  (void)p;
  (void)n;
  (void)m;
  return DG_UNIMPLEMENTED;
}
int dg_stream_feed(DGStream *s, const uint8_t *p, size_t n, size_t *u) {
  (void)s;
  (void)p;
  (void)n;
  (void)u;
  return DG_UNIMPLEMENTED;
}
int dg_stream_take(DGStream *s, DGSession *c, DGMessage *m) {
  (void)s;
  (void)c;
  (void)m;
  return DG_UNIMPLEMENTED;
}
int dg_stream_end(const DGStream *s) {
  (void)s;
  return DG_UNIMPLEMENTED;
}
