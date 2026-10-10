#include "com_http.h"
#include <string.h>
void http_init(HttpParser *p, const char *authority) {
  (void)authority;
  memset(p, 0, sizeof *p);
}
int http_feed(HttpParser *p, const void *v, size_t n, size_t *used) {
  (void)p;
  (void)v;
  (void)n;
  *used = 0;
  return HTTP_ERROR;
}
int http_eof(HttpParser *p) {
  (void)p;
  return HTTP_ERROR;
}
int http_response(const HttpRequest *r, const char *d, int c, void *o, size_t n,
                  size_t *z) {
  (void)r;
  (void)d;
  (void)c;
  (void)o;
  (void)n;
  *z = 0;
  return -1;
}
int http_expired(uint64_t n, uint64_t s, uint64_t p, uint64_t t, uint64_t i) {
  (void)n;
  (void)s;
  (void)p;
  (void)t;
  (void)i;
  return 0;
}
int http_drain(HttpWrite w, void *c, const void *v, size_t n, size_t *o) {
  (void)w;
  (void)c;
  (void)v;
  (void)n;
  (void)o;
  return -1;
}
