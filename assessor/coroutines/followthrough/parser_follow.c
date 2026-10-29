#include "parser_follow.h"
#ifndef CP_MUTANT
#define CP_MUTANT 0
#endif
struct co_match
{
    int status;
    size_t end;
    unsigned cut;
};

struct co_parse
{
    const struct cp_node *nodes;
    const unsigned char *input;
    size_t length;
    unsigned eof, budget;
    struct cp_diagnostic diagnostic;
};

static void expected(struct co_parse *p, size_t pos, unsigned char c)
{
    struct cp_diagnostic *d = &p->diagnostic;
    if (pos < d->offset)
    {
        return;
    }
    if (pos > d->offset)
    {
        d->offset = pos;
        d->count = d->truncated = 0;
    }
    for (unsigned i = 0; i < d->count; i++)
    {
        if (d->expected[i] == c)
        {
            return;
        }
    }
    unsigned i = 0;
    while (i < d->count && d->expected[i] < c)
    {
        i++;
    }
    if (d->count == 4)
    {
        d->truncated = 1;
        if (i == 4)
        {
            return;
        }
    }
    else
    {
        d->count++;
    }
    for (unsigned j = d->count - 1; j > i; j--)
    {
        d->expected[j] = d->expected[j - 1];
    }
    d->expected[i] = c;
}

static struct co_match parse(struct co_parse *p, unsigned index, size_t pos)
{
    if (!p->budget)
    {
        return (struct co_match){CO_RANGE, pos, 0};
    }
    p->budget--;
    p->diagnostic.steps++;
    const struct cp_node *n = &p->nodes[index];
    if (n->kind == CP_EMPTY)
    {
        return (struct co_match){CO_OK, pos, 0};
    }
    if (n->kind == CP_LITERAL)
    {
        if (pos < p->length && p->input[pos] == n->literal)
        {
            return (struct co_match){CO_OK, pos + 1, 0};
        }
        expected(p, pos, n->literal);
        return (struct co_match){pos == p->length && !p->eof && CP_MUTANT != 4
                                     ? CO_PENDING
                                     : CO_NO_MATCH,
                                 pos, 0};
    }
    struct co_match a = parse(p, n->left, pos);
    if (n->kind == CP_CUT)
    {
        if (a.status == CO_OK && CP_MUTANT != 1)
        {
            a.cut = 1;
        }
        return a;
    }
    if (n->kind == CP_CHOICE)
    {
        if (a.status != CO_NO_MATCH)
        {
            return a;
        }
        return parse(p, n->right, pos);
    }
    if (a.status)
    {
        return a;
    }
    struct co_match b = parse(p, n->right, a.end);
    b.cut |= a.cut;
    if (b.status == CO_NO_MATCH && b.cut)
    {
        b.status = CP_COMMITTED;
    }
    return b;
}

int cp_parse(const struct cp_node *n, size_t count, const unsigned char *in,
             size_t length, unsigned eof, unsigned budget, size_t *end,
             struct cp_diagnostic *out)
{
    if (!n || (!in && length) || !end || !out || eof > 1)
    {
        return CO_INVALID;
    }
    if (!count || count > 16 || length > 64)
    {
        return CO_RANGE;
    }
    for (size_t i = 0; i < count; i++)
    {
        unsigned k = n[i].kind;
        if (k > CP_EMPTY)
        {
            return CO_INVALID;
        }
        if (k == CP_SEQUENCE || k == CP_CHOICE || k == CP_CUT)
        {
            if (n[i].left <= i || n[i].left >= count)
            {
                return CO_INVALID;
            }
            if (k != CP_CUT && (n[i].right <= i || n[i].right >= count))
            {
                return CO_INVALID;
            }
        }
    }
    struct co_parse p = {n, in, length, eof, budget, {0}};
    struct co_match r = parse(&p, 0, 0);
    p.diagnostic.committed = r.cut;
    *out = p.diagnostic;
    if (r.status == CO_OK)
    {
        *end = r.end;
    }
    return r.status;
}

/* Direct left recursion only: E <- E '+' digit / digit.
 * The in-progress cell exposes the current seed, then strictly grows its end. */
static int digit(const unsigned char *in, size_t n, size_t p, unsigned mask,
                 int32_t *out)
{
    if (p >= n || in[p] < '0' || in[p] > '9' || !(mask & (1u << (in[p] - '0'))))
    {
        return 0;
    }
    *out = in[p] - '0';
    return 1;
}

static int apply(struct ce_memo *m, unsigned *budget, unsigned *calls)
{
    if (m->state)
    {
        return m->success;
    }
    if (!*budget)
    {
        return -1;
    }
    m->state = 1;
    m->success = 0;
    m->end = 0;
    for (;;)
    {
        if (!*budget)
        {
            m->state = 0;
            return -1;
        }
        (*budget)--;
        (*calls)++;
        size_t end = 0;
        int32_t value = 0, right = 0;
        int success = 0;
        if (apply(m, budget, calls) && m->end < m->length &&
            m->input[m->end] == '+' &&
            digit(m->input, m->length, m->end + 1, m->mask, &right))
        {
            success = 1;
            end = m->end + 2;
            value = m->value + right;
        }
        else if (digit(m->input, m->length, 0, m->mask, &value))
        {
            success = 1;
            end = 1;
        }
        if (!success || end <= m->end)
        {
            break;
        }
        m->success = 1;
        m->end = end;
        m->value = value;
        if (CP_MUTANT == 3)
        {
            break;
        }
    }
    m->state = 2;
    return m->success;
}

int cp_left_expression(struct ce_memo *m, const unsigned char *in, size_t n,
                       uint64_t inputrev, uint64_t envrev, unsigned mask,
                       unsigned budget, size_t *end, int32_t *value,
                       unsigned *calls)
{
    if (!m || (!in && n) || !end || !value || !calls || mask > 1023)
    {
        return CO_INVALID;
    }
    if (n > 64)
    {
        return CO_RANGE;
    }
    int same = m->valid && m->input == in && m->length == n;
    if (CP_MUTANT != 2)
    {
        same = same && m->input_revision == inputrev &&
               m->environment_revision == envrev && m->mask == mask;
    }
    if (!same)
    {
        *m = (struct ce_memo){.input = in,
                              .length = n,
                              .input_revision = inputrev,
                              .environment_revision = envrev,
                              .mask = mask,
                              .valid = 1};
    }
    *calls = 0;
    int r = apply(m, &budget, calls);
    if (r < 0)
    {
        m->valid = 0;
        return CO_RANGE;
    }
    if (!r || m->end != n)
    {
        return CO_NO_MATCH;
    }
    *end = m->end;
    *value = m->value;
    return CO_OK;
}
