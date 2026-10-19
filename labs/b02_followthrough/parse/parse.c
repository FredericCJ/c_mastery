#include "parse.h"
int bf_parse_reset(BfParse *s, unsigned b) {
  (void)s;
  (void)b;
  return BF_UNIMPLEMENTED;
}
int bf_parse_feed(BfParse *s, const unsigned char *p, size_t n) {
  (void)s;
  (void)p;
  (void)n;
  return BF_UNIMPLEMENTED;
}
int bf_parse_finish(BfParse *s, int32_t *out) {
  (void)s;
  (void)out;
  return BF_UNIMPLEMENTED;
}
int bf_parse_cancel(BfParse *s) {
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
