#include "lc.h"
unsigned lc_release(void) { return 0; }
LCStatus lc_info(unsigned a, unsigned char *b, size_t c) {
  (void)a;
  (void)b;
  (void)c;
  return LC_UNIMPLEMENTED;
}
LCBuffer *lc_create(size_t n) {
  (void)n;
  return 0;
}
LCStatus lc_append(LCBuffer *p, const unsigned char *s, size_t n) {
  (void)p;
  (void)s;
  (void)n;
  return LC_UNIMPLEMENTED;
}
const unsigned char *lc_view(const LCBuffer *p, size_t *n) {
  (void)p;
  (void)n;
  return 0;
}
LCStatus lc_copy(const LCBuffer *p, unsigned char *s, size_t n, size_t *w) {
  (void)p;
  (void)s;
  (void)n;
  (void)w;
  return LC_UNIMPLEMENTED;
}
LCStatus lc_extent(LCElements e, size_t w, LCBytes b, LCBytes *o) {
  (void)e;
  (void)w;
  (void)b;
  (void)o;
  return LC_UNIMPLEMENTED;
}
void lc_release_buffer(LCBuffer *p) { (void)p; }
LCStatus lc_sync(LCDone d, void *p) {
  (void)d;
  (void)p;
  return LC_UNIMPLEMENTED;
}
LCOperation *lc_start(LCDone d, void *p) {
  (void)d;
  (void)p;
  return 0;
}
LCStatus lc_cancel(LCOperation *p) {
  (void)p;
  return LC_UNIMPLEMENTED;
}
LCStatus lc_poll(LCOperation *p) {
  (void)p;
  return LC_UNIMPLEMENTED;
}
LCStatus lc_ack(LCOperation *p) {
  (void)p;
  return LC_UNIMPLEMENTED;
}
LCStatus lc_release_operation(LCOperation *p) {
  (void)p;
  return LC_UNIMPLEMENTED;
}
