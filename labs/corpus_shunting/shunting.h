#ifndef CORPUS_SHUNTING_H
#define CORPUS_SHUNTING_H
#include "parsing.h"
/* Caller-owned state. Inspect fields, but mutate only through these operations.
 * All live state/input/output objects must be disjoint. No dynamic allocation. */
typedef struct { unsigned pushes, reductions, peak_operators, peak_values; } SyCost;
typedef struct { unsigned kind; size_t offset; } SyOperator;
typedef struct {
    SyOperator operators[64];
    unsigned values[64], operators_used, values_used;
    PdTree tree;
    SyCost cost;
    unsigned right_power, operator_capacity, node_capacity;
    unsigned want_operand, done, tokens;
} SyState;
int sy_begin(unsigned right_power, unsigned operator_capacity,
             unsigned node_capacity, SyState *out);
/* A successful non-EOF token returns PD_PENDING; EOF returns PD_OK. On any
 * other status the complete prior state remains byte-for-byte unchanged. */
int sy_push(SyState *state, const PdToken *token, PdDiagnostic *diagnostic);
int sy_result(const SyState *state, PdTree *out, SyCost *cost);
int sy_parse(const PdTokens *tokens, unsigned right_power,
             unsigned operator_capacity, unsigned node_capacity,
             PdTree *out, SyCost *cost, PdDiagnostic *diagnostic);
#endif
