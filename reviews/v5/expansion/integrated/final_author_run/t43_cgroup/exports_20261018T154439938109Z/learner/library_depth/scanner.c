#include "scanner.h"
enum ls_status ls_record_parse(const unsigned char *p, size_t n, int escaped,
                               struct ls_record *out) {
  (void)p;
  (void)n;
  (void)escaped;
  (void)out;
  return LS_NOT_IMPLEMENTED;
}
enum ls_status ls_line(FILE *f, unsigned char *p, size_t cap, size_t *n) {
  (void)f;
  (void)p;
  (void)cap;
  (void)n;
  return LS_NOT_IMPLEMENTED;
}
