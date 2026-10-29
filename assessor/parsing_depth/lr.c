#include "parsing.h"
#ifndef PD_MUTANT
#define PD_MUTANT 0
#endif
static unsigned base(const struct lr_grammar *g, unsigned p)
{
    unsigned b = 0;
    for (unsigned i = 0; i < p; ++i)
    {
        b += g->productions[i].length + 1;
    }
    return b;
}

static uint64_t closure(const struct lr_grammar *g, uint64_t set)
{
    uint64_t old;
    do
    {
        old = set;
        for (unsigned p = 0; p < g->count; ++p)
        {
            struct lr_production r = g->productions[p];
            unsigned b = base(g, p);
            for (unsigned dot = 0; dot < r.length; ++dot)
            {
                if ((set & (UINT64_C(1) << (b + dot))) && r.rhs[dot] >= 8)
                {
                    for (unsigned q = 0; q < g->count; ++q)
                    {
                        if (g->productions[q].lhs == r.rhs[dot])
                        {
                            set |= UINT64_C(1) << base(g, q);
                        }
                    }
                }
            }
        }
    } while (old != set);
    return set;
}

static uint64_t transition(const struct lr_grammar *g, uint64_t set,
                           unsigned symbol)
{
    uint64_t out = 0;
    for (unsigned p = 0; p < g->count; ++p)
    {
        struct lr_production r = g->productions[p];
        unsigned b = base(g, p);
        for (unsigned dot = 0; dot < r.length; ++dot)
        {
            if (r.rhs[dot] == symbol && (set & (UINT64_C(1) << (b + dot))))
            {
                out |= UINT64_C(1) << (b + dot + 1);
            }
        }
    }
    return closure(g, out);
}

static int action(int *slot, int value)
{
    if (*slot && *slot != value)
    {
        if (PD_MUTANT != 3)
        {
            return PD_CONFLICT;
        }
    }
    *slot = value;
    return PD_OK;
}

int lr_build(const struct lr_grammar *g, struct lr_table *out,
             struct pd_diagnostic *d)
{
    if (!g || !out || !d)
    {
        return PD_INVALID;
    }
    *d = (struct pd_diagnostic){0};
    if (!g->count || g->count > 12)
    {
        return PD_INVALID;
    }
    if (g->productions[0].lhs != 12 || g->productions[0].length != 1 ||
        g->productions[0].rhs[0] < 8 || g->productions[0].rhs[0] > 11)
    {
        return PD_INVALID;
    }
    unsigned declared = 0;
    for (unsigned p = 1; p < g->count; ++p)
    {
        unsigned lhs = g->productions[p].lhs;
        if (lhs < 8 || lhs > 11)
        {
            return PD_INVALID;
        }
        declared |= 1u << (lhs - 8);
    }
    if (!(declared & (1u << (g->productions[0].rhs[0] - 8))))
    {
        return PD_INVALID;
    }
    for (unsigned p = 0; p < g->count; ++p)
    {
        struct lr_production r = g->productions[p];
        if (r.length > 4)
        {
            return PD_INVALID;
        }
        for (unsigned i = 0; i < r.length; ++i)
        {
            if (r.rhs[i] == 7 || r.rhs[i] > 11 ||
                (r.rhs[i] >= 8 && !(declared & (1u << (r.rhs[i] - 8)))))
            {
                return PD_INVALID;
            }
        }
    }
    struct lr_table t = {0};
    t.grammar = *g;
    for (unsigned i = 0; i < 32; ++i)
    {
        for (unsigned j = 0; j < 5; ++j)
        {
            t.go[i][j] = -1;
        }
    }
    unsigned changed = 1;
    while (changed)
    {
        changed = 0;
        for (unsigned p = 0; p < g->count; ++p)
        {
            struct lr_production r = g->productions[p];
            unsigned lhs = r.lhs - 8, old = t.first[lhs], all = 1;
            for (unsigned i = 0; i < r.length; ++i)
            {
                unsigned s = r.rhs[i];
                if (s < 8)
                {
                    t.first[lhs] |= 1u << s;
                    all = 0;
                    break;
                }
                t.first[lhs] |= t.first[s - 8];
                if (!t.nullable[s - 8])
                {
                    all = 0;
                    break;
                }
            }
            if (all && !t.nullable[lhs])
            {
                t.nullable[lhs] = 1;
                changed = 1;
            }
            if (t.first[lhs] != old)
            {
                changed = 1;
            }
        }
    }
    t.follow[g->productions[0].rhs[0] - 8] = 1u << 7;
    changed = 1;
    while (changed)
    {
        changed = 0;
        for (unsigned p = 0; p < g->count; ++p)
        {
            struct lr_production r = g->productions[p];
            for (unsigned i = 0; i < r.length; ++i)
            {
                if (r.rhs[i] >= 8)
                {
                    unsigned s = r.rhs[i] - 8, old = t.follow[s], all = 1;
                    for (unsigned j = i + 1; j < r.length; ++j)
                    {
                        unsigned next = r.rhs[j];
                        if (next < 8)
                        {
                            t.follow[s] |= 1u << next;
                            all = 0;
                            break;
                        }
                        t.follow[s] |= t.first[next - 8];
                        if (!t.nullable[next - 8])
                        {
                            all = 0;
                            break;
                        }
                    }
                    if (all)
                    {
                        t.follow[s] |= t.follow[r.lhs - 8];
                    }
                    if (t.follow[s] != old)
                    {
                        changed = 1;
                    }
                }
            }
        }
    }
    t.count = 1;
    t.items[0] = closure(g, 1);
    for (unsigned i = 0; i < t.count; ++i)
    {
        for (unsigned symbol = 0; symbol < 12; ++symbol)
        {
            if (symbol == 7)
            {
                continue;
            }
            uint64_t next = transition(g, t.items[i], symbol);
            if (!next)
            {
                continue;
            }
            unsigned j = 0;
            while (j < t.count && t.items[j] != next)
            {
                ++j;
            }
            if (j == t.count)
            {
                if (t.count == 32)
                {
                    return PD_RANGE;
                }
                t.items[t.count++] = next;
            }
            if (symbol < 8)
            {
                int rc = action(&t.action[i][symbol], (int)j + 1);
                if (rc)
                {
                    d->offset = i;
                    d->expected = symbol;
                    return rc;
                }
            }
            else
            {
                t.go[i][symbol - 8] = (int)j;
            }
        }
        for (unsigned p = 0; p < g->count; ++p)
        {
            struct lr_production r = g->productions[p];
            if (!(t.items[i] & (UINT64_C(1) << (base(g, p) + r.length))))
            {
                continue;
            }
            unsigned follow = p ? t.follow[r.lhs - 8] : (1u << 7);
            if (PD_MUTANT == 4 && p)
            {
                follow = 255;
            }
            for (unsigned symbol = 0; symbol < 8; ++symbol)
            {
                if (follow & (1u << symbol))
                {
                    int rc = action(&t.action[i][symbol], p ? -(int)p : 99);
                    if (rc)
                    {
                        d->offset = i;
                        d->expected = symbol;
                        return rc;
                    }
                }
            }
        }
    }
    *out = t;
    return PD_OK;
}

int lr_parse(const struct lr_table *t, const unsigned char *in, size_t n,
             unsigned budget, struct lr_tree *out, struct pd_diagnostic *d)
{
    if (!t || (!in && n) || !out || !d || !t->count || t->count > 32)
    {
        return PD_INVALID;
    }
    *d = (struct pd_diagnostic){0};
    if (n > 64)
    {
        return PD_RANGE;
    }
    for (size_t i = 0; i < n; ++i)
    {
        if (in[i] >= 7)
        {
            return PD_INVALID;
        }
    }
    /* A table is a successful lr_build value, retained without mutation. */
    unsigned states[129] = {0}, nodes[128], depth = 0;
    size_t pos = 0;
    struct lr_tree tree = {0};
    for (;;)
    {
        if (!budget)
        {
            return PD_FUEL;
        }
        --budget;
        ++d->steps;
        d->offset = pos;
        unsigned symbol = pos < n ? in[pos] : 7;
        int a = t->action[states[depth]][symbol];
        if (a == 99)
        {
            if (pos != n || depth != 1)
            {
                return PD_INVALID;
            }
            tree.root = nodes[0];
            *out = tree;
            return PD_OK;
        }
        if (!a)
        {
            d->expected = symbol;
            return PD_NO_MATCH;
        }
        if (tree.count == 128)
        {
            return PD_RANGE;
        }
        unsigned node = tree.count++;
        if (a > 0)
        {
            if (pos == n || depth == 128)
            {
                return PD_INVALID;
            }
            tree.nodes[node] = (struct lr_node){.symbol = symbol,
                                                .production = 255,
                                                .begin = pos,
                                                .end = pos + 1};
            nodes[depth] = node;
            states[++depth] = (unsigned)(a - 1);
            ++pos;
        }
        else
        {
            unsigned p = (unsigned)(-a);
            struct lr_production r = t->grammar.productions[p];
            if (depth < r.length)
            {
                return PD_INVALID;
            }
            struct lr_node value = {.symbol = r.lhs,
                                    .production = p,
                                    .count = r.length,
                                    .begin = pos,
                                    .end = pos};
            for (unsigned j = 0; j < r.length; ++j)
            {
                value.children[j] = nodes[depth - r.length + j];
            }
            if (r.length)
            {
                value.begin = tree.nodes[value.children[0]].begin;
                value.end = tree.nodes[value.children[r.length - 1]].end;
            }
            depth -= r.length;
            int go = t->go[states[depth]][r.lhs - 8];
            if (go < 0)
            {
                return PD_INVALID;
            }
            tree.nodes[node] = value;
            nodes[depth] = node;
            states[++depth] = (unsigned)go;
        }
    }
}
