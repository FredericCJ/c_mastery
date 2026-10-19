#include "wire_depth.h"
int rpw_decode(const unsigned char *a, size_t b, RpwLimits c, RpwDoc *d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return RPW_UNIMPLEMENTED;
}
int rpw_encode(const RpwDoc *a, RpwLimits b, unsigned char *c, size_t d,
               size_t *e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return RPW_UNIMPLEMENTED;
}
int rpw_update(RpwDoc *a, RpwLimits b, uint32_t c, int32_t d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return RPW_UNIMPLEMENTED;
}
int rpw_project(const RpwDoc *a, RpwLimits b, RPMessage *c) {
  (void)a;
  (void)b;
  (void)c;
  return RPW_UNIMPLEMENTED;
}
uint64_t rpw_binary64(uint64_t a) { return a; }
int rpw_utf8(const unsigned char *a, size_t b) {
  (void)a;
  (void)b;
  return RPW_UNIMPLEMENTED;
}
int rpw_stream_init(RpwStream *a, RpwLimits b) {
  (void)a;
  (void)b;
  return RPW_UNIMPLEMENTED;
}
int rpw_feed(RpwStream *a, const unsigned char *b, size_t c, size_t *d,
             RpwDoc *e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return RPW_UNIMPLEMENTED;
}
int rpw_eof(RpwStream *a) {
  (void)a;
  return RPW_UNIMPLEMENTED;
}
int rpw_cancel(RpwStream *a) {
  (void)a;
  return RPW_UNIMPLEMENTED;
}
int rpw_frame(const RpwDoc *a, RpwLimits b, unsigned char *c, size_t d,
              size_t *e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return RPW_UNIMPLEMENTED;
}
