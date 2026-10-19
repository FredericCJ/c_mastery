#include "unicode.h"
enum lu_status lu_decode(struct lu_state *s, const unsigned char *b, size_t n,
                         uint32_t *o, size_t c, enum lu_policy p,
                         struct lu_result *r) {
  (void)s;
  (void)b;
  (void)n;
  (void)o;
  (void)c;
  (void)p;
  (void)r;
  return LU_NOT_IMPLEMENTED;
}
enum lu_status lu_finish(struct lu_state *s, uint32_t *o, size_t c,
                         enum lu_policy p, struct lu_result *r) {
  (void)s;
  (void)o;
  (void)c;
  (void)p;
  (void)r;
  return LU_NOT_IMPLEMENTED;
}
enum lu_status lu_encode(uint32_t u, unsigned char *o, size_t c, size_t *n) {
  (void)u;
  (void)o;
  (void)c;
  (void)n;
  return LU_NOT_IMPLEMENTED;
}
enum lu_status lu_utf16(uint32_t u, uint16_t *o, size_t c, size_t *n) {
  (void)u;
  (void)o;
  (void)c;
  (void)n;
  return LU_NOT_IMPLEMENTED;
}
