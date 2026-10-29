#ifndef CORPUS_SHUNTING_H
#define CORPUS_SHUNTING_H
#include "parsing.h"

/* Caller-owned state. Inspect fields, but mutate only through these operations.
 * All live state/input/output objects must be disjoint. No dynamic allocation. */
struct sy_cost
{
    unsigned pushes, reductions, peak_operators, peak_values;
};

struct sy_operator
{
    unsigned kind;
    size_t offset;
};

struct sy_state
{
    struct sy_operator operators[64];
    unsigned values[64], operators_used, values_used;
    struct pd_tree tree;
    struct sy_cost cost;
    unsigned right_power, operator_capacity, node_capacity;
    unsigned want_operand, done, tokens;
};

int sy_begin(unsigned right_power, unsigned operator_capacity,
             unsigned node_capacity, struct sy_state *out);
/* A successful non-EOF token returns PD_PENDING; EOF returns PD_OK. On any
 * other status the complete prior state remains byte-for-byte unchanged. */
int sy_push(struct sy_state *state, const struct pd_token *token,
            struct pd_diagnostic *diagnostic);
int sy_result(const struct sy_state *state, struct pd_tree *out,
              struct sy_cost *cost);
int sy_parse(const struct pd_tokens *tokens, unsigned right_power,
             unsigned operator_capacity, unsigned node_capacity,
             struct pd_tree *out, struct sy_cost *cost,
             struct pd_diagnostic *diagnostic);
#endif
