#include "parsing.h"
/* Implement the supplied contracts; no assessor implementation is shipped. */
int pd_lex(const unsigned char *text, size_t length, PdTokens *out,
           PdDiagnostic *diagnostic) {
  (void)text;
  (void)length;
  (void)out;
  (void)diagnostic;
  return PD_INVALID;
}
int pd_pratt(const PdTokens *tokens, unsigned right_power, PdTree *out,
             PdDiagnostic *diagnostic) {
  (void)tokens;
  (void)right_power;
  (void)out;
  (void)diagnostic;
  return PD_INVALID;
}
int pd_evaluate(const PdTree *tree, int32_t *out) {
  (void)tree;
  (void)out;
  return PD_INVALID;
}
int ps_parse(const PsNode *nodes, size_t count, const unsigned char *input,
             size_t length, unsigned eof, unsigned budget, PsResult *out,
             PdDiagnostic *diagnostic) {
  (void)nodes;
  (void)count;
  (void)input;
  (void)length;
  (void)eof;
  (void)budget;
  (void)out;
  (void)diagnostic;
  return PD_INVALID;
}
int rx_compile(const unsigned char *pattern, size_t length, RxNfa *out,
               PdDiagnostic *diagnostic) {
  (void)pattern;
  (void)length;
  (void)out;
  (void)diagnostic;
  return PD_INVALID;
}
int rx_match(const RxNfa *nfa, const unsigned char *text, size_t length,
             unsigned search, unsigned *out) {
  (void)nfa;
  (void)text;
  (void)length;
  (void)search;
  (void)out;
  return PD_INVALID;
}
int rx_determinize(const RxNfa *nfa, unsigned capacity, RxDfa *out) {
  (void)nfa;
  (void)capacity;
  (void)out;
  return PD_INVALID;
}
int rx_dfa_match(const RxDfa *dfa, const unsigned char *text, size_t length,
                 unsigned *out) {
  (void)dfa;
  (void)text;
  (void)length;
  (void)out;
  return PD_INVALID;
}
int lr_build(const LrGrammar *grammar, LrTable *out, PdDiagnostic *diagnostic) {
  (void)grammar;
  (void)out;
  (void)diagnostic;
  return PD_INVALID;
}
int lr_parse(const LrTable *table, const unsigned char *tokens, size_t length,
             unsigned budget, LrTree *out, PdDiagnostic *diagnostic) {
  (void)table;
  (void)tokens;
  (void)length;
  (void)budget;
  (void)out;
  (void)diagnostic;
  return PD_INVALID;
}
