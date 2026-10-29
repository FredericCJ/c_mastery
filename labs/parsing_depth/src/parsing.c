#include "parsing.h"

/* Implement the supplied contracts; no assessor implementation is shipped. */
int pd_lex(const unsigned char *text, size_t length, struct pd_tokens *out,
           struct pd_diagnostic *diagnostic)
{
    (void)text;
    (void)length;
    (void)out;
    (void)diagnostic;
    return PD_INVALID;
}

int pd_pratt(const struct pd_tokens *tokens, unsigned right_power,
             struct pd_tree *out, struct pd_diagnostic *diagnostic)
{
    (void)tokens;
    (void)right_power;
    (void)out;
    (void)diagnostic;
    return PD_INVALID;
}

int pd_evaluate(const struct pd_tree *tree, int32_t *out)
{
    (void)tree;
    (void)out;
    return PD_INVALID;
}

int ps_parse(const struct ps_node *nodes, size_t count,
             const unsigned char *input, size_t length, unsigned eof,
             unsigned budget, struct ps_result *out,
             struct pd_diagnostic *diagnostic)
{
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

int rx_compile(const unsigned char *pattern, size_t length, struct rx_nfa *out,
               struct pd_diagnostic *diagnostic)
{
    (void)pattern;
    (void)length;
    (void)out;
    (void)diagnostic;
    return PD_INVALID;
}

int rx_match(const struct rx_nfa *nfa, const unsigned char *text, size_t length,
             unsigned search, unsigned *out)
{
    (void)nfa;
    (void)text;
    (void)length;
    (void)search;
    (void)out;
    return PD_INVALID;
}

int rx_determinize(const struct rx_nfa *nfa, unsigned capacity,
                   struct rx_dfa *out)
{
    (void)nfa;
    (void)capacity;
    (void)out;
    return PD_INVALID;
}

int rx_dfa_match(const struct rx_dfa *dfa, const unsigned char *text,
                 size_t length, unsigned *out)
{
    (void)dfa;
    (void)text;
    (void)length;
    (void)out;
    return PD_INVALID;
}

int lr_build(const struct lr_grammar *grammar, struct lr_table *out,
             struct pd_diagnostic *diagnostic)
{
    (void)grammar;
    (void)out;
    (void)diagnostic;
    return PD_INVALID;
}

int lr_parse(const struct lr_table *table, const unsigned char *tokens,
             size_t length, unsigned budget, struct lr_tree *out,
             struct pd_diagnostic *diagnostic)
{
    (void)table;
    (void)tokens;
    (void)length;
    (void)budget;
    (void)out;
    (void)diagnostic;
    return PD_INVALID;
}
