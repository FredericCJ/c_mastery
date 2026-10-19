#include "stream.h"
int nw_http_pump(NwRx *s, HttpParser *p, size_t budget, size_t *used) {
  (void)s;
  (void)p;
  (void)budget;
  (void)used;
  return HTTP_ERROR;
}
