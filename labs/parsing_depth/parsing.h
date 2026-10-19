#ifndef PARSING_DEPTH_H
#define PARSING_DEPTH_H
#include <stddef.h>
#include <stdint.h>
enum {
  PD_OK = 0,
  PD_NO_MATCH = 1,
  PD_PENDING = 2,
  PD_RANGE = 3,
  PD_INVALID = 4,
  PD_CONFLICT = 5,
  PD_FUEL = 6,
  PD_EMPTY_LOOP = 7,
  PD_DOMAIN = 8
};
typedef struct {
  size_t offset;
  unsigned expected, steps;
} PdDiagnostic;
/* Tokens: integer0,+1,*2,(3,)4,-5,^6,EOF7. */
typedef struct {
  unsigned kind;
  int32_t value;
  size_t offset, length, line, column;
} PdToken;
typedef struct {
  PdToken tokens[65];
  size_t count;
} PdTokens;
typedef struct {
  unsigned kind, left, right;
  int32_t value;
  size_t offset;
} PdExpr;
typedef struct {
  PdExpr nodes[64];
  unsigned count, root;
} PdTree;
int pd_lex(const unsigned char *text, size_t length, PdTokens *out,
           PdDiagnostic *diagnostic);
int pd_pratt(const PdTokens *tokens, unsigned right_power, PdTree *out,
             PdDiagnostic *diagnostic);
int pd_evaluate(const PdTree *tree, int32_t *out);

/* Scannerless expression DAG: all child indices strictly exceed their parent.
 */
enum {
  PS_BYTE = 0,
  PS_RANGE = 1,
  PS_ANY = 2,
  PS_EMPTY = 3,
  PS_SEQ = 4,
  PS_CHOICE = 5,
  PS_STAR = 6,
  PS_NOT = 7,
  PS_CAPTURE = 8
};
typedef struct {
  unsigned kind, left, right, low, high;
} PsNode;
typedef struct {
  unsigned tag;
  size_t begin, end;
} PsCapture;
typedef struct {
  size_t end;
  unsigned count;
  PsCapture captures[32];
} PsResult;
int ps_parse(const PsNode *nodes, size_t count, const unsigned char *input,
             size_t length, unsigned eof, unsigned budget, PsResult *out,
             PdDiagnostic *diagnostic);

enum { RX_BYTE = 0, RX_EPSILON = 1, RX_SPLIT = 2, RX_ACCEPT = 3 };
typedef struct {
  unsigned kind, byte, next, other;
} RxState;
typedef struct {
  RxState states[64];
  unsigned count, start;
} RxNfa;
typedef struct {
  uint64_t subsets[16];
  unsigned next[16][2], accept[16], count;
} RxDfa;
int rx_compile(const unsigned char *pattern, size_t length, RxNfa *out,
               PdDiagnostic *diagnostic);
int rx_match(const RxNfa *nfa, const unsigned char *text, size_t length,
             unsigned search, unsigned *out);
int rx_determinize(const RxNfa *nfa, unsigned capacity, RxDfa *out);
int rx_dfa_match(const RxDfa *dfa, const unsigned char *text, size_t length,
                 unsigned *out);

/* Terminals0..6, EOF7, nonterminals8..11; production0 must be12->start. */
typedef struct {
  unsigned lhs, length, rhs[4];
} LrProduction;
typedef struct {
  unsigned count;
  LrProduction productions[12];
} LrGrammar;
typedef struct {
  LrGrammar grammar;
  uint64_t items[32];
  unsigned count;
  int action[32][8], go[32][5];
  unsigned first[5], follow[5], nullable[5];
} LrTable;
typedef struct {
  unsigned symbol, production, count, children[4];
  size_t begin, end;
} LrNode;
typedef struct {
  LrNode nodes[128];
  unsigned count, root;
} LrTree;
int lr_build(const LrGrammar *grammar, LrTable *out, PdDiagnostic *diagnostic);
int lr_parse(const LrTable *table, const unsigned char *tokens, size_t length,
             unsigned budget, LrTree *out, PdDiagnostic *diagnostic);
#endif
