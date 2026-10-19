#include "shunting.h"
/* Deliberate learner scaffold. Implement the stated contract from primitives. */
int sy_begin(unsigned r,unsigned o,unsigned n,SyState *s) {
    (void)r;(void)o;(void)n;(void)s;return PD_INVALID;
}
int sy_push(SyState *s,const PdToken *t,PdDiagnostic *d) {
    (void)s;(void)t;(void)d;return PD_INVALID;
}
int sy_result(const SyState *s,PdTree *t,SyCost *c) {
    (void)s;(void)t;(void)c;return PD_INVALID;
}
int sy_parse(const PdTokens *t,unsigned r,unsigned o,unsigned n,PdTree *v,SyCost *c,PdDiagnostic *d) {
    (void)t;(void)r;(void)o;(void)n;(void)v;(void)c;(void)d;return PD_INVALID;
}
