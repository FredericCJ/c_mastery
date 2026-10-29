#include "parsing.h"
#ifndef PD_MUTANT
#define PD_MUTANT 0
#endif
struct pd_fragment
{
    unsigned start, end;
};

struct pd_compile
{
    const unsigned char *pattern;
    size_t length, pos;
    struct rx_nfa nfa;
    int status;
    struct pd_diagnostic *d;
};

static unsigned state(struct pd_compile *p, unsigned kind, unsigned byte,
                      unsigned next, unsigned other)
{
    if (p->nfa.count == 64)
    {
        p->status = PD_RANGE;
        return 0;
    }
    unsigned id = p->nfa.count++;
    p->nfa.states[id] = (struct rx_state){kind, byte, next, other};
    return id;
}
static struct pd_fragment expression(struct pd_compile *p, unsigned depth);

static struct pd_fragment atom(struct pd_compile *p, unsigned depth)
{
    struct pd_fragment f = {0, 0};
    if (p->status)
    {
        return f;
    }
    p->d->offset = p->pos;
    ++p->d->steps;
    if (p->pos == p->length)
    {
        p->status = PD_NO_MATCH;
        return f;
    }
    unsigned c = p->pattern[p->pos++];
    if (c == '(')
    {
        if (depth == 24)
        {
            p->status = PD_RANGE;
            return f;
        }
        f = expression(p, depth + 1);
        if (p->pos < p->length && p->pattern[p->pos] == ')')
        {
            ++p->pos;
        }
        else
        {
            p->status = PD_NO_MATCH;
        }
    }
    else if (c == 'a' || c == 'b' || c == '~')
    {
        f.start = state(p, c == '~' ? RX_EPSILON : RX_BYTE, c, 0, 0);
        f.end = state(p, RX_EPSILON, 0, 0, 0);
        p->nfa.states[f.start].next = f.end;
    }
    else
    {
        p->status = PD_NO_MATCH;
    }
    while (!p->status && p->pos < p->length && p->pattern[p->pos] == '*')
    {
        ++p->pos;
        unsigned end = state(p, RX_EPSILON, 0, 0, 0),
                 start = state(p, RX_SPLIT, 0, f.start, end);
        p->nfa.states[f.end].next = start;
        f = (struct pd_fragment){start, end};
    }
    return f;
}

static struct pd_fragment concatenate(struct pd_compile *p, unsigned depth)
{
    struct pd_fragment f = atom(p, depth);
    while (!p->status && p->pos < p->length && p->pattern[p->pos] != ')' &&
           p->pattern[p->pos] != '|')
    {
        struct pd_fragment next = atom(p, depth);
        p->nfa.states[f.end].next = next.start;
        f.end = next.end;
    }
    return f;
}

static struct pd_fragment expression(struct pd_compile *p, unsigned depth)
{
    struct pd_fragment f = concatenate(p, depth);
    while (!p->status && p->pos < p->length && p->pattern[p->pos] == '|')
    {
        ++p->pos;
        struct pd_fragment b = concatenate(p, depth);
        unsigned end = state(p, RX_EPSILON, 0, 0, 0);
        unsigned start = state(p, RX_SPLIT, 0, f.start, b.start);
        p->nfa.states[f.end].next = end;
        p->nfa.states[b.end].next = end;
        f = (struct pd_fragment){start, end};
    }
    return f;
}

int rx_compile(const unsigned char *s, size_t n, struct rx_nfa *out,
               struct pd_diagnostic *d)
{
    if ((!s && n) || !out || !d)
    {
        return PD_INVALID;
    }
    *d = (struct pd_diagnostic){0};
    if (n > 24)
    {
        return PD_RANGE;
    }
    struct pd_compile p = {.pattern = s, .length = n, .d = d};
    struct pd_fragment f = expression(&p, 0);
    if (p.status)
    {
        return p.status;
    }
    if (p.pos != n)
    {
        d->offset = p.pos;
        return PD_NO_MATCH;
    }
    p.nfa.states[f.end] = (struct rx_state){RX_ACCEPT, 0, 0, 0};
    p.nfa.start = f.start;
    *out = p.nfa;
    return PD_OK;
}

static int valid(const struct rx_nfa *n)
{
    if (!n || !n->count || n->count > 64 || n->start >= n->count)
    {
        return 0;
    }
    for (unsigned i = 0; i < n->count; ++i)
    {
        struct rx_state s = n->states[i];
        if (s.kind > RX_ACCEPT)
        {
            return 0;
        }
        if (s.kind != RX_ACCEPT && s.next >= n->count)
        {
            return 0;
        }
        if (s.kind == RX_SPLIT && s.other >= n->count)
        {
            return 0;
        }
        if (s.kind == RX_BYTE && s.byte != 'a' && s.byte != 'b')
        {
            return 0;
        }
    }
    return 1;
}

static uint64_t closure(const struct rx_nfa *n, uint64_t set)
{
    unsigned queue[64], head = 0, tail = 0;
    for (unsigned i = 0; i < n->count; ++i)
    {
        if (set & (UINT64_C(1) << i))
        {
            queue[tail++] = i;
        }
    }
    while (head < tail)
    {
        struct rx_state s = n->states[queue[head++]];
        unsigned edges[2] = {s.next, s.other};
        unsigned count = s.kind == RX_SPLIT     ? (PD_MUTANT == 1 ? 1 : 2)
                         : s.kind == RX_EPSILON ? 1
                                                : 0;
        for (unsigned i = 0; i < count; ++i)
        {
            uint64_t bit = UINT64_C(1) << edges[i];
            if (!(set & bit))
            {
                set |= bit;
                queue[tail++] = edges[i];
            }
        }
    }
    return set;
}

static uint64_t step(const struct rx_nfa *n, uint64_t set, unsigned c)
{
    uint64_t next = 0;
    for (unsigned i = 0; i < n->count; ++i)
    {
        if ((set & (UINT64_C(1) << i)) && n->states[i].kind == RX_BYTE &&
            n->states[i].byte == c)
        {
            next |= UINT64_C(1) << n->states[i].next;
        }
    }
    return closure(n, next);
}

static unsigned accepts(const struct rx_nfa *n, uint64_t set)
{
    for (unsigned i = 0; i < n->count; ++i)
    {
        if (n->states[i].kind == RX_ACCEPT && (set & (UINT64_C(1) << i)))
        {
            return 1;
        }
    }
    return 0;
}

int rx_match(const struct rx_nfa *n, const unsigned char *s, size_t length,
             unsigned search, unsigned *out)
{
    if (!valid(n) || (!s && length) || !out || search > 1)
    {
        return PD_INVALID;
    }
    if (length > 256)
    {
        return PD_RANGE;
    }
    uint64_t initial = closure(n, UINT64_C(1) << n->start), set = initial;
    unsigned found = search && accepts(n, set);
    for (size_t i = 0; i < length; ++i)
    {
        set = step(n, set, s[i]);
        if (search)
        {
            found |= accepts(n, set);
            set |= initial;
        }
    }
    *out = search ? found : accepts(n, set);
    return PD_OK;
}

int rx_determinize(const struct rx_nfa *n, unsigned capacity,
                   struct rx_dfa *out)
{
    if (!valid(n) || !out || !capacity || capacity > 16)
    {
        return PD_INVALID;
    }
    struct rx_dfa d = {0};
    d.count = 1;
    d.subsets[0] = closure(n, UINT64_C(1) << n->start);
    for (unsigned i = 0; i < d.count; ++i)
    {
        d.accept[i] = accepts(n, d.subsets[i]);
        for (unsigned c = 0; c < 2; ++c)
        {
            uint64_t set = step(n, d.subsets[i], (unsigned)'a' + c);
            unsigned j = 0;
            while (j < d.count && d.subsets[j] != set)
            {
                ++j;
            }
            if (j == d.count)
            {
                if (d.count == capacity)
                {
                    return PD_RANGE;
                }
                d.subsets[d.count++] = set;
            }
            d.next[i][c] = j;
        }
    }
    *out = d;
    return PD_OK;
}

int rx_dfa_match(const struct rx_dfa *d, const unsigned char *s, size_t n,
                 unsigned *out)
{
    if (!d || (!s && n) || !out || !d->count || d->count > 16)
    {
        return PD_INVALID;
    }
    if (n > 256)
    {
        return PD_RANGE;
    }
    for (unsigned i = 0; i < d->count; ++i)
    {
        if (d->accept[i] > 1 || d->next[i][0] >= d->count ||
            d->next[i][1] >= d->count)
        {
            return PD_INVALID;
        }
    }
    unsigned state_id = 0;
    for (size_t i = 0; i < n; ++i)
    {
        if (s[i] != 'a' && s[i] != 'b')
        {
            *out = 0;
            return PD_OK;
        }
        state_id = d->next[state_id][s[i] - 'a'];
    }
    *out = PD_MUTANT == 2 ? d->accept[0] : d->accept[state_id];
    return PD_OK;
}
