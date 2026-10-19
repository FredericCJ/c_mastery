#include "handoff.h"
/* Intentionally unfinished learner implementation. */
int xi_init(XiQueue *q, unsigned n) {
  (void)q;
  (void)n;
  return XI_UNIMPLEMENTED;
}
int xi_reserve(XiQueue *q) {
  (void)q;
  return XI_UNIMPLEMENTED;
}
int xi_fill(XiQueue *q, const XiRecord *r) {
  (void)q;
  (void)r;
  return XI_UNIMPLEMENTED;
}
int xi_commit(XiQueue *q) {
  (void)q;
  return XI_UNIMPLEMENTED;
}
int xi_cancel(XiQueue *q) {
  (void)q;
  return XI_UNIMPLEMENTED;
}
int xi_take(XiQueue *q, XiRecord *r) {
  (void)q;
  (void)r;
  return XI_UNIMPLEMENTED;
}
int xi_release(XiQueue *q) {
  (void)q;
  return XI_UNIMPLEMENTED;
}
int xi_push(XiQueue *q, const XiRecord *r) {
  (void)q;
  (void)r;
  return XI_UNIMPLEMENTED;
}
int xi_pop(XiQueue *q, XiRecord *r) {
  (void)q;
  (void)r;
  return XI_UNIMPLEMENTED;
}
int xi_service_init(XiService *s, XiQueue *q) {
  (void)s;
  (void)q;
  return XI_UNIMPLEMENTED;
}
int xi_capture_begin(XiService *s, uint32_t a, uint32_t b,
                     const unsigned char *p, uint32_t n) {
  (void)s;
  (void)a;
  (void)b;
  (void)p;
  (void)n;
  return XI_UNIMPLEMENTED;
}
int xi_capture_end(XiService *s) {
  (void)s;
  return XI_UNIMPLEMENTED;
}
void xi_latest_init(XiLatest *q) { (void)q; }
int xi_latest_push(XiLatest *q, const XiRecord *r) {
  (void)q;
  (void)r;
  return XI_UNIMPLEMENTED;
}
int xi_latest_pop(XiLatest *q, XiRecord *r) {
  (void)q;
  (void)r;
  return XI_UNIMPLEMENTED;
}
int xi_mask_init(XiMask *s, XfPriorityConfig c, unsigned a, unsigned b) {
  (void)s;
  (void)c;
  (void)a;
  (void)b;
  return XI_UNIMPLEMENTED;
}
int xi_mask_enter(XiMask *s, unsigned n, uint32_t *t) {
  (void)s;
  (void)n;
  (void)t;
  return XI_UNIMPLEMENTED;
}
int xi_mask_leave(XiMask *s, uint32_t t) {
  (void)s;
  (void)t;
  return XI_UNIMPLEMENTED;
}
