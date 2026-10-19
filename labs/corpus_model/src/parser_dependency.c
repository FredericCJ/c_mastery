/* Replace through CM_PARSER_SOURCE with your accepted X-T15-012 provider. */
#include "parsing.h"
int pd_lex(const unsigned char *s, size_t n, PdTokens *t, PdDiagnostic *d) {
  (void)s;
  (void)n;
  (void)t;
  (void)d;
  return PD_INVALID;
}
int pd_pratt(const PdTokens *t, unsigned r, PdTree *o, PdDiagnostic *d) {
  (void)t;
  (void)r;
  (void)o;
  (void)d;
  return PD_INVALID;
}
int pd_evaluate(const PdTree *t, int32_t *o) {
  (void)t;
  (void)o;
  return PD_INVALID;
}
