#include "wire_depth.h"
int rpw_decode(const unsigned char *a, size_t b, struct rpw_limits c, struct rpw_doc *d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return RPW_UNIMPLEMENTED;
}
int rpw_encode(const struct rpw_doc *a, struct rpw_limits b, unsigned char *c, size_t d,
               size_t *e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return RPW_UNIMPLEMENTED;
}
int rpw_update(struct rpw_doc *a, struct rpw_limits b, uint32_t c, int32_t d) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  return RPW_UNIMPLEMENTED;
}
int rpw_project(const struct rpw_doc *a, struct rpw_limits b, struct rp_message *c) {
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
int rpw_stream_init(struct rpw_stream *a, struct rpw_limits b) {
  (void)a;
  (void)b;
  return RPW_UNIMPLEMENTED;
}
int rpw_feed(struct rpw_stream *a, const unsigned char *b, size_t c, size_t *d,
             struct rpw_doc *e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return RPW_UNIMPLEMENTED;
}
int rpw_eof(struct rpw_stream *a) {
  (void)a;
  return RPW_UNIMPLEMENTED;
}
int rpw_cancel(struct rpw_stream *a) {
  (void)a;
  return RPW_UNIMPLEMENTED;
}
int rpw_frame(const struct rpw_doc *a, struct rpw_limits b, unsigned char *c, size_t d,
              size_t *e) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)e;
  return RPW_UNIMPLEMENTED;
}
