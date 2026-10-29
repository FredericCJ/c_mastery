#include "parsing.h"
#include <limits.h>
#ifndef PD_MUTANT
#define PD_MUTANT 0
#endif
int pd_lex(const unsigned char *s, size_t n, struct pd_tokens *out,
           struct pd_diagnostic *d)
{
    struct pd_tokens t = {0};
    size_t p = 0, line = 1, col = 1;
    if ((!s && n) || !out || !d)
        return PD_INVALID;
    *d = (struct pd_diagnostic){0};
    if (n > 256)
        return PD_RANGE;
    while (p < n)
    {
        size_t begin = p, startcol = col;
        unsigned kind;
        int32_t value = 0;
        if (s[p] == ' ' || s[p] == '\t' || s[p] == '\n')
        {
            if (s[p] == '\n')
            {
                ++line;
                col = 1;
            }
            else
                ++col;
            ++p;
            continue;
        }
        if (t.count == 64)
        {
            d->offset = p;
            return PD_RANGE;
        }
        if (s[p] >= '0' && s[p] <= '9')
        {
            kind = 0;
            do
            {
                unsigned digit = s[p] - '0';
                if (value > (INT32_MAX - (int32_t)digit) / 10)
                {
                    d->offset = p;
                    return PD_RANGE;
                }
                value = value * 10 + (int32_t)digit;
                ++p;
                ++col;
            } while (p < n && s[p] >= '0' && s[p] <= '9');
        }
        else
        {
            switch (s[p])
            {
                case '+':
                    kind = 1;
                    break;
                case '*':
                    kind = 2;
                    break;
                case '(':
                    kind = 3;
                    break;
                case ')':
                    kind = 4;
                    break;
                case '-':
                    kind = 5;
                    break;
                case '^':
                    kind = 6;
                    break;
                default:
                    d->offset = p;
                    d->expected = 256;
                    return PD_NO_MATCH;
            }
            ++p;
            ++col;
        }
        t.tokens[t.count++] =
            (struct pd_token){kind, value, begin, p - begin, line, startcol};
    }
    t.tokens[t.count++] = (struct pd_token){7, 0, n, 0, line, col};
    *out = t;
    return PD_OK;
}

struct pd_pratt
{
    const struct pd_tokens *t;
    struct pd_tree tree;
    size_t pos;
    unsigned right;
    int status;
    struct pd_diagnostic *d;
};

static unsigned node(struct pd_pratt *p, unsigned kind, unsigned left,
                     unsigned right, int32_t value, size_t off)
{
    if (p->tree.count == 64)
    {
        p->status = PD_RANGE;
        return 0;
    }
    unsigned i = p->tree.count++;
    p->tree.nodes[i] = (struct pd_expr){kind, left, right, value, off};
    return i;
}

static unsigned power(unsigned kind)
{
    return kind == 1 || kind == 5 ? 10 : kind == 2 ? 20 : kind == 6 ? 40 : 0;
}

static unsigned expression(struct pd_pratt *p, unsigned minimum, unsigned depth)
{
    if (p->status)
        return 0;
    if (depth > 64)
    {
        p->status = PD_RANGE;
        return 0;
    }
    const struct pd_token *t = &p->t->tokens[p->pos];
    unsigned left = 0;
    p->d->steps++;
    p->d->offset = t->offset;
    if (t->kind == 0)
    {
        ++p->pos;
        left = node(p, 0, 0, 0, t->value, t->offset);
    }
    else if (t->kind == 5)
    {
        ++p->pos;
        unsigned child = expression(p, 30, depth + 1);
        left = node(p, 5, child, 0, 0, t->offset);
    }
    else if (t->kind == 3)
    {
        ++p->pos;
        left = expression(p, 0, depth + 1);
        if (!p->status && p->t->tokens[p->pos].kind == 4)
            ++p->pos;
        else
        {
            p->status = PD_NO_MATCH;
            p->d->offset = p->t->tokens[p->pos].offset;
            p->d->expected = 4;
        }
    }
    else
    {
        p->status = PD_NO_MATCH;
        p->d->expected = 0;
        return 0;
    }
    while (!p->status && power(p->t->tokens[p->pos].kind) > minimum)
    {
        const struct pd_token *op = &p->t->tokens[p->pos++];
        unsigned bp = power(op->kind);
        if (op->kind == 6 && p->right && PD_MUTANT != 5)
            --bp;
        unsigned right = expression(p, bp, depth + 1);
        left =
            node(p, op->kind == 5 ? 8 : op->kind, left, right, 0, op->offset);
    }
    return left;
}

int pd_pratt(const struct pd_tokens *t, unsigned right, struct pd_tree *out,
             struct pd_diagnostic *d)
{
    if (!t || !out || !d || right > 1)
        return PD_INVALID;
    *d = (struct pd_diagnostic){0};
    if (!t->count || t->count > 65 || t->tokens[t->count - 1].kind != 7)
        return PD_INVALID;
    for (size_t i = 0; i < t->count - 1; ++i)
        if (t->tokens[i].kind >= 7 ||
            (t->tokens[i].kind == 0 && t->tokens[i].value < 0))
            return PD_INVALID;
    struct pd_pratt p = {.t = t, .right = right, .d = d};
    p.tree.root = expression(&p, 0, 0);
    if (p.status)
        return p.status;
    if (p.pos != t->count - 1)
    {
        d->offset = t->tokens[p.pos].offset;
        d->expected = 7;
        return PD_NO_MATCH;
    }
    *out = p.tree;
    return PD_OK;
}

static int checked(int64_t value, int32_t *out)
{
    if (value < INT32_MIN || value > INT32_MAX)
        return PD_RANGE;
    *out = (int32_t)value;
    return PD_OK;
}

int pd_evaluate(const struct pd_tree *t, int32_t *out)
{
    int32_t values[64];
    if (!t || !out || !t->count || t->count > 64 || t->root >= t->count)
        return PD_INVALID;
    for (unsigned i = 0; i < t->count; ++i)
    {
        const struct pd_expr *n = &t->nodes[i];
        int rc = PD_OK;
        if (n->kind == 0)
        {
            values[i] = n->value;
            continue;
        }
        if (n->left >= i || (n->kind != 5 && n->right >= i))
            return PD_INVALID;
        int32_t a = values[n->left], b = n->kind == 5 ? 0 : values[n->right];
        if (n->kind == 1)
            rc = checked((int64_t)a + b, &values[i]);
        else if (n->kind == 2)
            rc = checked((int64_t)a * b, &values[i]);
        else if (n->kind == 8)
            rc = checked((int64_t)a - b, &values[i]);
        else if (n->kind == 5)
            rc = checked(-(int64_t)a, &values[i]);
        else if (n->kind == 6)
        {
            if (b < 0 || b > 31)
                return PD_DOMAIN;
            int32_t value = 1;
            for (int32_t j = 0; j < b; ++j)
            {
                rc = checked((int64_t)value * a, &value);
                if (rc)
                    break;
            }
            values[i] = value;
        }
        else
            return PD_INVALID;
        if (rc && PD_MUTANT != 6)
            return rc;
        if (rc)
            values[i] = 0;
    }
    *out = values[t->root];
    return PD_OK;
}
