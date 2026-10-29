#include "text.h"
int tc_recover(const unsigned char *s, size_t n, unsigned mode, unsigned fuel,
               struct tc_batch *out) {
  (void)s;
  (void)n;
  (void)mode;
  (void)fuel;
  (void)out;
  return TC_INVALID;
}
int tc_layout(const struct tc_doc *d, unsigned width, unsigned fuel, struct tc_text *out) {
  (void)d;
  (void)width;
  (void)fuel;
  (void)out;
  return TC_INVALID;
}
int tc_expression_doc(const struct pd_tree *t, struct tc_doc *out) {
  (void)t;
  (void)out;
  return TC_INVALID;
}
int tc_template_compile(const unsigned char *s, size_t n, struct tc_program *out) {
  (void)s;
  (void)n;
  (void)out;
  return TC_INVALID;
}
int tc_template_render(const struct tc_program *b, unsigned n, unsigned e,
                       const struct tc_model *m, unsigned html, unsigned fuel,
                       struct tc_text *out) {
  (void)b;
  (void)n;
  (void)e;
  (void)m;
  (void)html;
  (void)fuel;
  (void)out;
  return TC_INVALID;
}
