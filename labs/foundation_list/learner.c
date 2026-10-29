#include "list.h"
enum ft_status ft_tokens(const char *const *p, size_t n, int c, struct ft_result *o,
                   struct ft_errors *e) {
  (void)p;
  (void)n;
  (void)c;
  (void)o;
  (void)e;
  return FT_NOT_IMPLEMENTED;
}
void ft_stream_init(struct ft_stream *s) { (void)s; }
enum ft_status ft_stream_feed(struct ft_stream *s, const unsigned char *p, size_t n) {
  (void)s;
  (void)p;
  (void)n;
  return FT_NOT_IMPLEMENTED;
}
enum ft_status ft_stream_finish(struct ft_stream *s, struct ft_result *o) {
  (void)s;
  (void)o;
  return FT_NOT_IMPLEMENTED;
}
const char *ft_name(enum ft_status s) {
  (void)s;
  return "not-implemented";
}
