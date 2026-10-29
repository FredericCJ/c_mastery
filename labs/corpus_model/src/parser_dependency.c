/* Replace through CM_PARSER_SOURCE with your accepted X-T15-012 provider. */
#include "parsing.h"

int pd_lex(const unsigned char *s, size_t n, struct pd_tokens *t,
           struct pd_diagnostic *d)
{
    (void)s;
    (void)n;
    (void)t;
    (void)d;
    return PD_INVALID;
}

int pd_pratt(const struct pd_tokens *t, unsigned r, struct pd_tree *o,
             struct pd_diagnostic *d)
{
    (void)t;
    (void)r;
    (void)o;
    (void)d;
    return PD_INVALID;
}

int pd_evaluate(const struct pd_tree *t, int32_t *o)
{
    (void)t;
    (void)o;
    return PD_INVALID;
}
