#include "stream.h"
int nw_http_pump(struct nw_rx *s, struct http_parser *p, size_t budget, size_t *used) {
  (void)s;
  (void)p;
  (void)budget;
  (void)used;
  return HTTP_ERROR;
}
