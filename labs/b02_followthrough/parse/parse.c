#include "parse.h"
int bf_parse_reset(struct bf_parse *s, unsigned b) {
  (void)s;
  (void)b;
  return BF_UNIMPLEMENTED;
}
int bf_parse_feed(struct bf_parse *s, const unsigned char *p, size_t n) {
  (void)s;
  (void)p;
  (void)n;
  return BF_UNIMPLEMENTED;
}
int bf_parse_finish(struct bf_parse *s, int32_t *out) {
  (void)s;
  (void)out;
  return BF_UNIMPLEMENTED;
}
int bf_parse_cancel(struct bf_parse *s) {
  (void)s;
  return BF_UNIMPLEMENTED;
}
int bf_parse_radix(const unsigned char *p, size_t n, unsigned b, int32_t *out) {
  (void)p;
  (void)n;
  (void)b;
  (void)out;
  return BF_UNIMPLEMENTED;
}
