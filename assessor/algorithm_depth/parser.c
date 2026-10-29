#include "algorithms.h"
#ifndef AD_MUTANT
#define AD_MUTANT 0
#endif
static enum ad_status error(struct ad_diagnostic *d, enum ad_status status,
                            size_t offset)
{
    *d = (struct ad_diagnostic){offset, status};
    return status;
}

enum ad_status ad_lex(const unsigned char *p, size_t n, struct ad_token *out,
                      size_t capacity, size_t *count,
                      struct ad_diagnostic *diag)
{
    if (!diag)
    {
        return AD_INVALID;
    }
    if (n > 64 || (n && !p) || !count)
        return error(diag, AD_INVALID, 0);
    struct ad_token tokens[65];
    size_t used = 0, at = 0;
    while (at < n)
    {
        unsigned char c = p[at];
        if (c == 32 || c == 9 || c == 10 || c == 13)
        {
            ++at;
            continue;
        }
        struct ad_token token = {.offset = at, .length = 1};
        if (c >= '0' && c <= '9')
        {
            uint32_t value = 0;
            token.kind = AD_T_NUMBER;
            size_t begin = at;
            while (at < n && p[at] >= '0' && p[at] <= '9')
            {
                unsigned digit = p[at] - '0';
                if (value > (INT32_MAX - digit) / 10)
                    return error(diag, AD_OVERFLOW, at);
                value = value * 10 + digit;
                ++at;
            }
            token.length = at - begin;
            token.value = (int32_t)value;
        }
        else
        {
            ++at;
            switch (c)
            {
                case '+':
                    token.kind = AD_T_PLUS;
                    break;
                case '*':
                    token.kind = AD_T_STAR;
                    break;
                case '^':
                    token.kind = AD_T_POWER;
                    break;
                case '(':
                    token.kind = AD_T_LEFT;
                    break;
                case ')':
                    token.kind = AD_T_RIGHT;
                    break;
                default:
                    if (c >= 'a' && c <= 'z')
                    {
                        token.kind = AD_T_NAME;
                        token.name = c;
                    }
                    else
                        return error(diag, AD_INVALID, at - 1);
            }
        }
        tokens[used++] = token;
    }
    tokens[used++] = (struct ad_token){.kind = AD_T_END, .offset = n};
    if (used > capacity || !out)
        return error(diag, AD_SPACE, n);
    for (size_t i = 0; i < used; ++i)
        out[i] = tokens[i];
    *count = used;
    *diag = (struct ad_diagnostic){n, AD_OK};
    return AD_OK;
}

enum ad_status ad_token_copy(const unsigned char *p, size_t n,
                             struct ad_token token, unsigned char *out,
                             size_t capacity, size_t *count)
{
    if ((n && !p) || !count || token.offset > n ||
        token.length > n - token.offset)
        return AD_INVALID;
    if (token.length > capacity || (token.length && !out))
        return AD_SPACE;
    for (size_t i = 0; i < token.length; ++i)
        out[i] = p[token.offset + i];
    *count = token.length;
    return AD_OK;
}

struct ad_parser
{
    const struct ad_token *t;
    size_t n, at;
    unsigned depth_limit, extensions, capacity, fail_at;
    struct ad_ast ast;
    struct ad_diagnostic *diag;
    enum ad_status status;
};

static unsigned node(struct ad_parser *p, struct ad_node value)
{
    if (p->ast.count >= p->capacity || p->ast.count == p->fail_at)
    {
        p->status = error(p->diag, AD_SPACE, value.offset);
        return 0;
    }
    unsigned id = p->ast.count++;
    p->ast.nodes[id] = value;
    return id;
}

static unsigned expression(struct ad_parser *p, unsigned minimum,
                           unsigned depth)
{
    if (p->status != AD_OK)
    {
        return 0;
    }
    if (depth > p->depth_limit)
    {
        p->status = error(p->diag, AD_SPACE, p->t[p->at].offset);
        return 0;
    }
    struct ad_token token = p->t[p->at];
    unsigned left = 0;
    if (token.kind == AD_T_NUMBER || (p->extensions && token.kind == AD_T_NAME))
    {
        ++p->at;
        left = node(p, (struct ad_node){.kind = token.kind,
                                        .value = token.value,
                                        .name = token.name,
                                        .offset = token.offset});
    }
    else if (token.kind == AD_T_LEFT)
    {
        ++p->at;
        left = expression(p, 1, depth + 1);
        if (p->status != AD_OK)
            return 0;
        if (p->t[p->at].kind != AD_T_RIGHT)
        {
            p->status = error(p->diag, AD_INVALID, p->t[p->at].offset);
            return 0;
        }
        ++p->at;
    }
    else
    {
        p->status = error(p->diag, AD_INVALID, token.offset);
        return 0;
    }
    while (p->status == AD_OK)
    {
        token = p->t[p->at];
        unsigned precedence = token.kind == AD_T_PLUS                       ? 1
                              : token.kind == AD_T_STAR                     ? 2
                              : (p->extensions && token.kind == AD_T_POWER) ? 3
                                                                            : 0;
        if (precedence < minimum)
            break;
        ++p->at;
        unsigned right = expression(
            p,
            precedence + (token.kind == AD_T_POWER && AD_MUTANT != 8 ? 0 : 1),
            depth + 1);
        if (p->status != AD_OK)
            return 0;
        left = node(p, (struct ad_node){.kind = token.kind,
                                        .left = left,
                                        .right = right,
                                        .offset = token.offset});
    }
    return left;
}

enum ad_status ad_parse(const struct ad_token *t, size_t n, unsigned depth,
                        unsigned extensions, unsigned capacity,
                        unsigned fail_at, struct ad_ast *out,
                        struct ad_diagnostic *diag)
{
    if (!diag)
    {
        return AD_INVALID;
    }
    if (!t || !n || n > 65 || !out || !depth || depth > 16 || extensions > 1 ||
        capacity > 32)
        return error(diag, AD_INVALID, 0);
    if (t[n - 1].kind != AD_T_END)
    {
        return error(diag, AD_INVALID, t[n - 1].offset);
    }
    for (size_t i = 0; i + 1 < n; ++i)
        if (t[i].kind == AD_T_END)
            return error(diag, AD_INVALID, t[i].offset);
    struct ad_parser p = {.t = t,
                          .n = n,
                          .depth_limit = depth,
                          .extensions = extensions,
                          .capacity = capacity,
                          .fail_at = fail_at,
                          .diag = diag,
                          .status = AD_OK};
    unsigned root = expression(&p, 1, 1);
    if (p.status != AD_OK)
        return p.status;
    if (p.at != n - 1)
        return error(diag, AD_INVALID, t[p.at].offset);
    p.ast.root = root;
    *out = p.ast;
    *diag = (struct ad_diagnostic){t[p.at].offset, AD_OK};
    return AD_OK;
}

enum ad_status ad_evaluate(const struct ad_ast *ast, ADLookup lookup,
                           void *context, int32_t *out,
                           struct ad_diagnostic *diag)
{
    if (!diag)
    {
        return AD_INVALID;
    }
    if (!ast || !out || !ast->count || ast->count > 32 ||
        ast->root >= ast->count)
        return error(diag, AD_INVALID, 0);
    int32_t values[32];
    for (unsigned i = 0; i < ast->count; ++i)
    {
        struct ad_node n = ast->nodes[i];
        if (n.kind == AD_T_NUMBER)
        {
            values[i] = n.value;
            continue;
        }
        if (n.kind == AD_T_NAME)
        {
            if (!lookup)
                return error(diag, AD_MISSING, n.offset);
            enum ad_status rc = lookup(context, n.name, &values[i]);
            if (rc != AD_OK)
                return error(diag, rc, n.offset);
            continue;
        }
        if (n.left >= i || n.right >= i)
            return error(diag, AD_INVALID, n.offset);
        int64_t a = values[n.left], b = values[n.right], result;
        if (n.kind == AD_T_PLUS)
            result = a + b;
        else if (n.kind == AD_T_STAR)
            result = a * b;
        else if (n.kind == AD_T_POWER)
        {
            if (b < 0 || b > 31)
                return error(diag, AD_INVALID, n.offset);
            result = 1;
            for (int64_t k = 0; k < b; ++k)
            {
                result *= a;
                if (result < INT32_MIN || result > INT32_MAX)
                {
                    return error(diag, AD_OVERFLOW, n.offset);
                }
            }
        }
        else
            return error(diag, AD_INVALID, n.offset);
        if (result < INT32_MIN || result > INT32_MAX)
        {
            return error(diag, AD_OVERFLOW, n.offset);
        }
        values[i] = (int32_t)result;
    }
    *out = values[ast->root];
    *diag = (struct ad_diagnostic){ast->nodes[ast->root].offset, AD_OK};
    return AD_OK;
}
