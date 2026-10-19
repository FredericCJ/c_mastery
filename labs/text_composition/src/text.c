#include "text.h"
int tc_recover(const unsigned char *s, size_t n, unsigned mode, unsigned fuel,
               TcBatch *out) {
  (void)s;
  (void)n;
  (void)mode;
  (void)fuel;
  (void)out;
  return TC_INVALID;
}
int tc_layout(const TcDoc *d, unsigned width, unsigned fuel, TcText *out) {
  (void)d;
  (void)width;
  (void)fuel;
  (void)out;
  return TC_INVALID;
}
int tc_expression_doc(const PdTree *t, TcDoc *out) {
  (void)t;
  (void)out;
  return TC_INVALID;
}
int tc_template_compile(const unsigned char *s, size_t n, TcProgram *out) {
  (void)s;
  (void)n;
  (void)out;
  return TC_INVALID;
}
int tc_template_render(const TcProgram *b, unsigned n, unsigned e,
                       const TcModel *m, unsigned html, unsigned fuel,
                       TcText *out) {
  (void)b;
  (void)n;
  (void)e;
  (void)m;
  (void)html;
  (void)fuel;
  (void)out;
  return TC_INVALID;
}
