#include "shunting.h"
/* Deliberate learner scaffold. Implement the stated contract from primitives. */
int sy_begin(unsigned r,unsigned o,unsigned n,struct sy_state *s) {
    (void)r;(void)o;(void)n;(void)s;return PD_INVALID;
}
int sy_push(struct sy_state *s,const struct pd_token *t,struct pd_diagnostic *d) {
    (void)s;(void)t;(void)d;return PD_INVALID;
}
int sy_result(const struct sy_state *s,struct pd_tree *t,struct sy_cost *c) {
    (void)s;(void)t;(void)c;return PD_INVALID;
}
int sy_parse(const struct pd_tokens *t,unsigned r,unsigned o,unsigned n,struct pd_tree *v,struct sy_cost *c,struct pd_diagnostic *d) {
    (void)t;(void)r;(void)o;(void)n;(void)v;(void)c;(void)d;return PD_INVALID;
}
