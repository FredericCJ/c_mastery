#include "list.h"
#include <limits.h>
_Static_assert(INT_MAX >= 524288 && '0' == 48 && '9' == 57,
               "explicit hosted ASCII int32-or-wider profile");
#ifndef FT_MUTANT
#define FT_MUTANT 0
#endif
/* Token states:0 untouched,1 sign only,2 digits,3 malformed. Arbitrarily many
 * leading zeros need no token buffer; range overflow is remembered, not added.
 */
static void push(struct ft_token *t, unsigned char c)
{
    if (t->state == 3)
    {
        return;
    }
    if (!t->state && (c == '+' || c == '-'))
    {
        t->negative = c == '-';
        t->state = 1;
        return;
    }
    if (c < '0' || c > '9')
    {
        if (FT_MUTANT != 5)
        {
            t->state = 3;
        }
        return;
    }
    t->state = 2;
    unsigned digit = c - '0', limit = t->negative ? 32768u : 32767u;
    if (t->magnitude > limit / 10 ||
        (t->magnitude == limit / 10 && digit > limit % 10))
    {
        t->range = 1;
    }
    if (!t->range)
    {
        t->magnitude = t->magnitude * 10 + digit;
    }
}

static enum ft_status token_value(struct ft_token t, int32_t *out)
{
    if (t.state != 2)
    {
        return FT_MALFORMED;
    }
    if (t.range)
    {
        return FT_RANGE;
    }
    *out = t.negative ? -(int32_t)t.magnitude : (int32_t)t.magnitude;
    return FT_OK;
}

static enum ft_status evaluate(const int32_t *values, size_t n,
                               struct ft_result *out)
{
    int32_t sum = 0;
    if (FT_MUTANT == 3)
    {
        out->count = n;
    }
    for (size_t i = 0; i < n; i++)
    {
        if (FT_MUTANT != 1 && ((values[i] > 0 && sum > 32767 - values[i]) ||
                               (values[i] < 0 && sum < -32768 - values[i])))
        {
            return FT_PREFIX;
        }
        /* Even the deliberately wrong final-only check is C-defined: <=16
         * values of magnitude32768 fit this explicit int32 arithmetic profile. */
        sum += values[i];
    }
    if (sum < -32768 || sum > 32767)
    {
        return FT_PREFIX;
    }
    *out = (struct ft_result){n, sum};
    return n ? FT_OK : FT_EMPTY;
}

static void error(struct ft_errors *e, size_t i, enum ft_status s)
{
    if (e->stored < FT_ERROR_CAPACITY)
    {
        e->items[e->stored++] = (struct ft_error){i, s};
    }
    ++e->total;
    if (FT_MUTANT == 4)
    {
        e->total = 1;
    }
}

enum ft_status ft_tokens(const char *const *tokens, size_t n, int collect,
                         struct ft_result *out, struct ft_errors *errors)
{
    if (!out || !errors || (n && !tokens) || (collect != 0 && collect != 1))
    {
        return FT_INVALID;
    }
    *errors = (struct ft_errors){0};
    int32_t values[FT_CAPACITY];
    enum ft_status first = FT_OK;
    for (size_t i = 0; i < n; i++)
    {
        struct ft_token token = {0};
        enum ft_status s = FT_OK;
        int32_t value = 0;
        if (!tokens[i])
        {
            s = FT_INVALID;
        }
        else
        {
            for (const unsigned char *p = (const unsigned char *)tokens[i]; *p;
                 p++)
            {
                push(&token, *p);
            }
            s = token_value(token, &value);
        }
        /* One diagnostic per token: lexical/range error precedes capacity. */
        if (s == FT_OK && i >= FT_CAPACITY)
        {
            s = FT_CAPACITY_ERROR;
        }
        if (s != FT_OK)
        {
            error(errors, i, s);
            if (first == FT_OK)
            {
                first = s;
            }
            if (!collect)
            {
                return s;
            }
        }
        else
        {
            values[i] = value;
        }
    }
    if (first != FT_OK)
    {
        return first;
    }
    enum ft_status s = evaluate(values, n, out);
    if (s == FT_PREFIX)
    {
        error(errors, n, FT_PREFIX); /* n marks aggregate arithmetic. */
    }
    return s;
}

void ft_stream_init(struct ft_stream *s)
{
    if (s)
    {
        *s = (struct ft_stream){0};
    }
}

static enum ft_status complete(struct ft_stream *s)
{
    int32_t value;
    enum ft_status status = token_value(s->token, &value);
    if (status == FT_OK && s->count == FT_CAPACITY)
    {
        status = FT_CAPACITY_ERROR;
    }
    if (status != FT_OK)
    {
        return s->status = status;
    }
    s->values[s->count++] = value;
    s->token = (struct ft_token){0};
    return FT_OK;
}

enum ft_status ft_stream_feed(struct ft_stream *s, const unsigned char *p,
                              size_t n)
{
    if (!s || (n && !p))
    {
        return FT_INVALID;
    }
    if (s->finished)
    {
        return FT_STATE;
    }
    if (s->status != FT_OK)
    {
        return s->status;
    }
    for (size_t i = 0; i < n; i++)
    {
        unsigned char c = p[i];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
        {
            if (s->token.state && complete(s) != FT_OK)
            {
                return s->status;
            }
        }
        else
        {
            push(&s->token, c);
        }
    }
    return FT_OK;
}

enum ft_status ft_stream_finish(struct ft_stream *s, struct ft_result *out)
{
    if (!s || !out)
    {
        return FT_INVALID;
    }
    if (s->finished)
    {
        return FT_STATE;
    }
    s->finished = 1;
    if (s->status != FT_OK)
    {
        return s->status;
    }
    if (s->token.state && FT_MUTANT != 2 && complete(s) != FT_OK)
    {
        return s->status;
    }
    return s->status = evaluate(s->values, s->count, out);
}

const char *ft_name(enum ft_status s)
{
    static const char *const names[] = {
        "ok",     "empty",   "malformed", "range",          "capacity",
        "prefix", "invalid", "state",     "not-implemented"};
    return (unsigned)s < sizeof names / sizeof names[0] ? names[s] : "unknown";
}
