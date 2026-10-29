#ifndef PARSING_DEPTH_H
#define PARSING_DEPTH_H
#include <stddef.h>
#include <stdint.h>

enum
{
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

struct pd_diagnostic
{
    size_t offset;
    unsigned expected, steps;
};

/* Tokens: integer0,+1,*2,(3,)4,-5,^6,EOF7. */
struct pd_token
{
    unsigned kind;
    int32_t value;
    size_t offset, length, line, column;
};

struct pd_tokens
{
    struct pd_token tokens[65];
    size_t count;
};

struct pd_expr
{
    unsigned kind, left, right;
    int32_t value;
    size_t offset;
};

struct pd_tree
{
    struct pd_expr nodes[64];
    unsigned count, root;
};

int pd_lex(const unsigned char *text, size_t length, struct pd_tokens *out,
           struct pd_diagnostic *diagnostic);
int pd_pratt(const struct pd_tokens *tokens, unsigned right_power,
             struct pd_tree *out, struct pd_diagnostic *diagnostic);
int pd_evaluate(const struct pd_tree *tree, int32_t *out);

/* Scannerless expression DAG: all child indices strictly exceed their parent.
 */
enum
{
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

struct ps_node
{
    unsigned kind, left, right, low, high;
};

struct ps_capture
{
    unsigned tag;
    size_t begin, end;
};

struct ps_result
{
    size_t end;
    unsigned count;
    struct ps_capture captures[32];
};

int ps_parse(const struct ps_node *nodes, size_t count,
             const unsigned char *input, size_t length, unsigned eof,
             unsigned budget, struct ps_result *out,
             struct pd_diagnostic *diagnostic);

enum
{
    RX_BYTE = 0,
    RX_EPSILON = 1,
    RX_SPLIT = 2,
    RX_ACCEPT = 3
};

struct rx_state
{
    unsigned kind, byte, next, other;
};

struct rx_nfa
{
    struct rx_state states[64];
    unsigned count, start;
};

struct rx_dfa
{
    uint64_t subsets[16];
    unsigned next[16][2], accept[16], count;
};

int rx_compile(const unsigned char *pattern, size_t length, struct rx_nfa *out,
               struct pd_diagnostic *diagnostic);
int rx_match(const struct rx_nfa *nfa, const unsigned char *text, size_t length,
             unsigned search, unsigned *out);
int rx_determinize(const struct rx_nfa *nfa, unsigned capacity,
                   struct rx_dfa *out);
int rx_dfa_match(const struct rx_dfa *dfa, const unsigned char *text,
                 size_t length, unsigned *out);

/* Terminals0..6, EOF7, nonterminals8..11; production0 must be12->start. */
struct lr_production
{
    unsigned lhs, length, rhs[4];
};

struct lr_grammar
{
    unsigned count;
    struct lr_production productions[12];
};

struct lr_table
{
    struct lr_grammar grammar;
    uint64_t items[32];
    unsigned count;
    int action[32][8], go[32][5];
    unsigned first[5], follow[5], nullable[5];
};

struct lr_node
{
    unsigned symbol, production, count, children[4];
    size_t begin, end;
};

struct lr_tree
{
    struct lr_node nodes[128];
    unsigned count, root;
};

int lr_build(const struct lr_grammar *grammar, struct lr_table *out,
             struct pd_diagnostic *diagnostic);
int lr_parse(const struct lr_table *table, const unsigned char *tokens,
             size_t length, unsigned budget, struct lr_tree *out,
             struct pd_diagnostic *diagnostic);
#endif
