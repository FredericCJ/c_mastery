#include "list.h"
FtStatus ft_tokens(const char *const *p, size_t n, int c, FtResult *o,
                   FtErrors *e) {
  (void)p;
  (void)n;
  (void)c;
  (void)o;
  (void)e;
  return FT_NOT_IMPLEMENTED;
}
void ft_stream_init(FtStream *s) { (void)s; }
FtStatus ft_stream_feed(FtStream *s, const unsigned char *p, size_t n) {
  (void)s;
  (void)p;
  (void)n;
  return FT_NOT_IMPLEMENTED;
}
FtStatus ft_stream_finish(FtStream *s, FtResult *o) {
  (void)s;
  (void)o;
  return FT_NOT_IMPLEMENTED;
}
const char *ft_name(FtStatus s) {
  (void)s;
  return "not-implemented";
}
