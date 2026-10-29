#include "shunting.h"
#ifndef SY_MUTANT
#define SY_MUTANT 0
#endif
static unsigned priority(unsigned kind) {
    if (kind == 1 || kind == 8) return 10;
    if (kind == 2) return 20;
    if (kind == 5) return SY_MUTANT == 2 ? 50 : 30;
    return 40;
}
static int fail(struct pd_diagnostic *d, size_t offset, unsigned expected, int code) {
    if (d) { d->offset = offset; d->expected = expected; }
    return code;
}
static int emit(struct sy_state *s, struct sy_operator op) {
    unsigned right, left, index;
    if (s->values_used < (op.kind == 5 ? 1u : 2u)) return PD_INVALID;
    if (s->tree.count == s->node_capacity) return PD_RANGE;
    right = s->values[--s->values_used];
    left = op.kind == 5 ? right : s->values[--s->values_used];
    if (SY_MUTANT == 3 && op.kind == 8) { unsigned t = left; left = right; right = t; }
    index = s->tree.count++;
    s->tree.nodes[index] = (struct pd_expr){op.kind, left, op.kind == 5 ? 0 : right, 0, op.offset};
    s->values[s->values_used++] = index;
    ++s->cost.reductions;
    return PD_OK;
}
static int operator_push(struct sy_state *s, unsigned kind, size_t offset) {
    if (s->operators_used == s->operator_capacity) return PD_RANGE;
    s->operators[s->operators_used++] = (struct sy_operator){kind, offset};
    ++s->cost.pushes;
    if (s->operators_used > s->cost.peak_operators) s->cost.peak_operators = s->operators_used;
    return PD_OK;
}
static int reduce(struct sy_state *s) {
    int rc = emit(s, s->operators[s->operators_used-1]);
    if (rc == PD_OK) --s->operators_used;
    return rc;
}
int sy_begin(unsigned right_power, unsigned operator_capacity,
             unsigned node_capacity, struct sy_state *out) {
    struct sy_state s = {0};
    if (!out || right_power > 1 || operator_capacity > 64 || node_capacity > 64) return PD_INVALID;
    s.right_power = right_power; s.operator_capacity = operator_capacity;
    s.node_capacity = node_capacity; s.want_operand = 1;
    *out = s;
    return PD_OK;
}
/* This internal routine mutates only private staging storage. Every caller
 * publishes it solely on success. Thus failed reductions may be destructive. */
static int consume(struct sy_state *s, const struct pd_token *t, struct pd_diagnostic *d) {
    unsigned k = t->kind;
    int rc;
    if (d) ++d->steps;
    if (s->done) return SY_MUTANT == 6 ? PD_OK : fail(d,t->offset,0,PD_INVALID);
    if (k > 7 || (k == 0 && t->value < 0 && SY_MUTANT != 7)) return fail(d,t->offset,0,PD_INVALID);
    if (k != 7 && s->tokens == 64) return fail(d,t->offset,1u<<7,PD_RANGE);
    if (k == 0) {
        if (!s->want_operand) return fail(d,t->offset,(1u<<1)|(1u<<2)|(1u<<7),PD_INVALID);
        if (s->tree.count == s->node_capacity) return fail(d,t->offset,0,PD_RANGE);
        s->tree.nodes[s->tree.count] = (struct pd_expr){0,0,0,t->value,t->offset};
        s->values[s->values_used++] = s->tree.count++;
        if (s->values_used > s->cost.peak_values) s->cost.peak_values = s->values_used;
        s->want_operand = 0;
    } else if (k == 3 || (k == 5 && s->want_operand)) {
        if (!s->want_operand) return fail(d,t->offset,1u<<1,PD_INVALID);
        rc = operator_push(s,k,t->offset);
        if (rc != PD_OK) return fail(d,t->offset,0,rc);
    } else if (k == 4 || k == 7) {
        if (s->want_operand) return fail(d,t->offset,(1u<<0)|(1u<<3)|(1u<<5),PD_INVALID);
        while (s->operators_used && s->operators[s->operators_used-1].kind != 3) {
            rc = reduce(s); if (rc != PD_OK) return fail(d,t->offset,0,rc);
        }
        if (k == 4) {
            if (!s->operators_used) return fail(d,t->offset,1u<<3,PD_INVALID);
            --s->operators_used;
        } else {
            if (s->operators_used || s->values_used != 1) return fail(d,t->offset,1u<<4,PD_INVALID);
            s->tree.root = s->values[0]; s->done = 1;
            return PD_OK;
        }
    } else {
        unsigned incoming = k == 5 ? 8 : k;
        unsigned right = incoming == 6 && s->right_power && SY_MUTANT != 1;
        if (s->want_operand) return fail(d,t->offset,(1u<<0)|(1u<<5)|(1u<<3),PD_INVALID);
        while (s->operators_used) {
            unsigned top = s->operators[s->operators_used-1].kind;
            if (top == 3 || priority(top) < priority(incoming) || (right && priority(top) == priority(incoming))) break;
            rc = reduce(s); if (rc != PD_OK) return fail(d,t->offset,0,rc);
        }
        rc = operator_push(s,incoming,t->offset);
        if (rc != PD_OK) return fail(d,t->offset,0,rc);
        s->want_operand = 1;
    }
    ++s->tokens;
    return PD_PENDING;
}
int sy_push(struct sy_state *state, const struct pd_token *token, struct pd_diagnostic *diagnostic) {
    struct sy_state next;
    int rc;
    if (!state || !token) return PD_INVALID;
    next = *state;
    if (diagnostic) *diagnostic = (struct pd_diagnostic){0,0,0};
    rc = consume(&next,token,diagnostic);
    if (rc == PD_OK || rc == PD_PENDING || SY_MUTANT == 5) *state = next;
    return rc;
}
int sy_result(const struct sy_state *state, struct pd_tree *out, struct sy_cost *cost) {
    if (!state || !out || !cost) return PD_INVALID;
    if (!state->done) return PD_PENDING;
    *out = state->tree; *cost = state->cost;
    return PD_OK;
}
int sy_parse(const struct pd_tokens *tokens, unsigned right_power,
             unsigned operator_capacity, unsigned node_capacity,
             struct pd_tree *out, struct sy_cost *cost, struct pd_diagnostic *diagnostic) {
    struct sy_state next;
    size_t i;
    int rc;
    if (!tokens || !out || !cost || !tokens->count || tokens->count > 65) return PD_INVALID;
    rc = sy_begin(right_power,operator_capacity,node_capacity,&next);
    if (rc != PD_OK) return rc;
    if (diagnostic) *diagnostic = (struct pd_diagnostic){0,0,0};
    for (i = 0; i < tokens->count; ++i) {
        if (tokens->tokens[i].kind == 7 && i+1 != tokens->count) return fail(diagnostic,tokens->tokens[i].offset,0,PD_INVALID);
        rc = consume(&next,&tokens->tokens[i],diagnostic);
        if (SY_MUTANT == 4) *out = next.tree;
        if (rc != PD_PENDING && rc != PD_OK) return rc;
    }
    if (!next.done) return fail(diagnostic,tokens->tokens[tokens->count-1].offset,1u<<7,PD_INVALID);
    return sy_result(&next,out,cost);
}
