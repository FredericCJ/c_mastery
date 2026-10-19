#include "grammars.h"
#include "parsing.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "PD_ASSERT:%d:%s\n", __LINE__, #x);                      \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static int regex_test(void) {
  RxNfa n, old;
  RxDfa d, keep;
  PdDiagnostic diag;
  unsigned out = 7;
  CHECK(rx_compile((const unsigned char *)"(a|b)*abb", 9, &n, &diag) == PD_OK);
  CHECK(rx_match(&n, (const unsigned char *)"aabb", 4, 0, &out) == PD_OK &&
        out == 1);
  CHECK(rx_match(&n, (const unsigned char *)"abbc", 4, 0, &out) == PD_OK &&
        out == 0);
  CHECK(rx_match(&n, (const unsigned char *)"cabbc", 5, 1, &out) == PD_OK &&
        out == 1);
  CHECK(rx_determinize(&n, 16, &d) == PD_OK);
  CHECK(rx_dfa_match(&d, (const unsigned char *)"aabb", 4, &out) == PD_OK &&
        out == 1);
  old = n;
  CHECK(rx_compile((const unsigned char *)"a|", 2, &n, &diag) == PD_NO_MATCH &&
        memcmp(&old, &n, sizeof n) == 0);
  memset(&keep, 0x5a, sizeof keep);
  d = keep;
  CHECK(rx_determinize(&n, 1, &d) == PD_RANGE &&
        memcmp(&keep, &d, sizeof d) == 0);
  CHECK(rx_compile((const unsigned char *)"(~|a)*", 6, &n, &diag) == PD_OK);
  CHECK(rx_match(&n, NULL, 0, 0, &out) == PD_OK && out == 1);
  CHECK(rx_match(&n, (const unsigned char *)"aaa", 3, 0, &out) == PD_OK &&
        out == 1);
  CHECK(rx_compile((const unsigned char *)"a**", 3, &n, &diag) == PD_OK);
  CHECK(rx_match(&n, (const unsigned char *)"b", 1, 0, &out) == PD_OK &&
        out == 0);
  return 0;
}
static int lr_test(void) {
  LrGrammar g = pd_grammar(0);
  LrTable t, old;
  LrTree tree, prior;
  PdDiagnostic d;
  CHECK(lr_build(&g, &t, &d) == PD_OK && t.count == 12);
  CHECK(t.follow[0] == ((1u << 1) | (1u << 4) | (1u << 7)));
  const unsigned char input[] = {0, 1, 0, 2, 0};
  CHECK(lr_parse(&t, input, 5, 100, &tree, &d) == PD_OK &&
        tree.nodes[tree.root].symbol == 8 && tree.nodes[tree.root].end == 5);
  prior = tree;
  CHECK(lr_parse(&t, input, 5, 1, &tree, &d) == PD_FUEL &&
        memcmp(&prior, &tree, sizeof tree) == 0);
  const unsigned char bad[] = {0, 1, 2};
  CHECK(lr_parse(&t, bad, 3, 100, &tree, &d) == PD_NO_MATCH && d.offset == 2);
  old = t;
  g = pd_grammar(1);
  CHECK(lr_build(&g, &t, &d) == PD_CONFLICT && memcmp(&old, &t, sizeof t) == 0);
  g = pd_grammar(2);
  CHECK(lr_build(&g, &t, &d) == PD_OK && t.nullable[1]);
  const unsigned char empty[] = {1}, many[] = {0, 0, 0, 1};
  CHECK(lr_parse(&t, empty, 1, 100, &tree, &d) == PD_OK);
  CHECK(lr_parse(&t, many, 4, 100, &tree, &d) == PD_OK);
  CHECK(lr_parse(&t, NULL, 0, 100, &tree, &d) == PD_NO_MATCH);
  return 0;
}
static int pratt_test(void) {
  PdTokens t, old;
  PdTree tree, prior;
  PdDiagnostic d;
  int32_t value = 123;
  CHECK(pd_lex((const unsigned char *)"2^3^2", 5, &t, &d) == PD_OK);
  CHECK(pd_pratt(&t, 1, &tree, &d) == PD_OK &&
        pd_evaluate(&tree, &value) == PD_OK && value == 512);
  CHECK(pd_pratt(&t, 0, &tree, &d) == PD_OK &&
        pd_evaluate(&tree, &value) == PD_OK && value == 64);
  CHECK(pd_lex((const unsigned char *)"-2^2", 4, &t, &d) == PD_OK);
  CHECK(pd_pratt(&t, 1, &tree, &d) == PD_OK &&
        pd_evaluate(&tree, &value) == PD_OK && value == -4);
  CHECK(pd_lex((const unsigned char *)"1+2*3", 5, &t, &d) == PD_OK);
  CHECK(pd_pratt(&t, 1, &tree, &d) == PD_OK &&
        pd_evaluate(&tree, &value) == PD_OK && value == 7);
  CHECK(pd_lex((const unsigned char *)"2147483647+1", 12, &t, &d) == PD_OK);
  CHECK(pd_pratt(&t, 1, &tree, &d) == PD_OK);
  value = 123;
  CHECK(pd_evaluate(&tree, &value) == PD_RANGE && value == 123);
  CHECK(pd_lex((const unsigned char *)"2^-1", 4, &t, &d) == PD_OK);
  CHECK(pd_pratt(&t, 1, &tree, &d) == PD_OK &&
        pd_evaluate(&tree, &value) == PD_DOMAIN && value == 123);
  CHECK(pd_lex((const unsigned char *)" \n12+3", 6, &t, &d) == PD_OK &&
        t.tokens[0].offset == 2 && t.tokens[0].line == 2 &&
        t.tokens[0].column == 1);
  old = t;
  CHECK(pd_lex((const unsigned char *)"2147483648", 10, &t, &d) == PD_RANGE &&
        memcmp(&old, &t, sizeof t) == 0);
  CHECK(pd_lex((const unsigned char *)"(1+)", 4, &t, &d) == PD_OK);
  prior = tree;
  CHECK(pd_pratt(&t, 1, &tree, &d) == PD_NO_MATCH &&
        memcmp(&prior, &tree, sizeof tree) == 0);
  return 0;
}
static int scannerless_test(void) {
  /* ((capture(a) x) / (a b)) then EOF: failed branch must discard capture. */
  const PsNode g[] = {{PS_SEQ, 1, 10, 0, 0},   {PS_CHOICE, 2, 7, 0, 0},
                      {PS_SEQ, 3, 6, 0, 0},    {PS_CAPTURE, 4, 0, 1, 0},
                      {PS_BYTE, 0, 0, 'a', 0}, {PS_EMPTY, 0, 0, 0, 0},
                      {PS_BYTE, 0, 0, 'x', 0}, {PS_SEQ, 8, 9, 0, 0},
                      {PS_BYTE, 0, 0, 'a', 0}, {PS_BYTE, 0, 0, 'b', 0},
                      {PS_NOT, 11, 0, 0, 0},   {PS_ANY, 0, 0, 0, 0}};
  PsResult out, prior;
  PdDiagnostic d;
  memset(&out, 0x5a, sizeof out);
  prior = out;
  CHECK(ps_parse(g, 12, (const unsigned char *)"ab", 2, 1, 100, &out, &d) ==
            PD_OK &&
        out.end == 2 && out.count == 0);
  prior = out;
  CHECK(ps_parse(g, 12, (const unsigned char *)"a", 1, 0, 100, &out, &d) ==
            PD_PENDING &&
        memcmp(&out, &prior, sizeof out) == 0);
  CHECK(ps_parse(g, 12, (const unsigned char *)"ab", 2, 0, 100, &out, &d) ==
        PD_PENDING);
  CHECK(ps_parse(g, 12, (const unsigned char *)"ab", 2, 1, 1, &out, &d) ==
        PD_FUEL);
  const PsNode loop[] = {{PS_STAR, 1, 0, 0, 0}, {PS_EMPTY, 0, 0, 0, 0}};
  CHECK(ps_parse(loop, 2, NULL, 0, 1, 10, &out, &d) == PD_EMPTY_LOOP);
  const PsNode capture[] = {{PS_SEQ, 1, 3, 0, 0},
                            {PS_NOT, 2, 0, 0, 0},
                            {PS_BYTE, 0, 0, 'b', 0},
                            {PS_CAPTURE, 4, 0, 9, 0},
                            {PS_BYTE, 0, 0, 'a', 0}};
  CHECK(ps_parse(capture, 5, (const unsigned char *)"a", 1, 1, 100, &out, &d) ==
            PD_OK &&
        out.count == 1 && out.captures[0].tag == 9 &&
        out.captures[0].begin == 0 && out.captures[0].end == 1);
  const PsNode many[] = {
      {PS_STAR, 1, 0, 0, 0}, {PS_CAPTURE, 2, 0, 1, 0}, {PS_BYTE, 0, 0, 'a', 0}};
  unsigned char bytes[33];
  memset(bytes, 'a', sizeof bytes);
  CHECK(ps_parse(many, 3, bytes, 32, 1, 200, &out, &d) == PD_OK &&
        out.count == 32);
  prior = out;
  CHECK(ps_parse(many, 3, bytes, 33, 1, 200, &out, &d) == PD_RANGE &&
        memcmp(&out, &prior, sizeof out) == 0);
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  int c = atoi(argv[1]);
  int rc = c == 1   ? regex_test()
           : c == 2 ? lr_test()
           : c == 3 ? pratt_test()
           : c == 4 ? scannerless_test()
                    : 2;
  if (!rc)
    puts("PASS parser mechanism assertions");
  return rc;
}
