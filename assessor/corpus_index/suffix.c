#include "index.h"
#ifndef CI_MUTANT
#define CI_MUTANT 0
#endif
static unsigned key(unsigned at, unsigned gap, unsigned n, const unsigned *rank)
{
    return at + gap <= n ? rank[at + gap] : 0;
}

static void sort(unsigned *out, const unsigned *in, unsigned n,
                 const unsigned *rank, unsigned gap)
{
    unsigned count[258];
    for (unsigned i = 0; i < 258; i++)
        count[i] = 0;
    for (unsigned i = 0; i <= n; i++)
        count[key(in[i], gap, n, rank)]++;
    unsigned sum = 0;
    for (unsigned i = 0; i < 258; i++)
    {
        unsigned v = count[i];
        count[i] = sum;
        sum += v;
    }
    for (unsigned i = 0; i <= n; i++)
    {
        unsigned k = key(in[i], gap, n, rank);
        out[count[k]++] = in[i];
    }
}

enum ci_status ci_suffix_build(struct ci_suffix *s, const unsigned char *bytes,
                               unsigned n)
{
    if (!s || (!bytes && n))
        return CI_INVALID;
    if (n > CI_CAP)
        return CI_RANGE;
    struct ci_suffix t;
    unsigned rank[CI_CAP + 1], next[CI_CAP + 1], tmp[CI_CAP + 1],
        inverse[CI_CAP + 1];
    t.n = n;
    for (unsigned i = 0; i < CI_CAP; i++)
        t.bytes[i] = i < n ? bytes[i] : 0;
    for (unsigned i = 0; i <= CI_CAP; i++)
    {
        t.order[i] = i;
        t.lcp[i] = 0;
        rank[i] = i < n ? (unsigned)bytes[i] + 1 : 0;
    }
    for (unsigned gap = 1; gap <= n; gap *= 2)
    {
        sort(tmp, t.order, n, rank, gap);
        sort(t.order, tmp, n, rank, 0);
        unsigned r = 0;
        next[t.order[0]] = 0;
        for (unsigned i = 1; i <= n; i++)
        {
            unsigned a = t.order[i - 1], b = t.order[i];
            if (rank[a] != rank[b] ||
                (CI_MUTANT != 7 &&
                 key(a, gap, n, rank) != key(b, gap, n, rank)))
                r++;
            next[b] = r;
        }
        for (unsigned i = 0; i <= n; i++)
            rank[i] = next[i];
        if (r == n)
            break;
    }
    for (unsigned i = 0; i <= n; i++)
        inverse[t.order[i]] = i;
    unsigned h = 0;
    for (unsigned i = 0; i <= n; i++)
    {
        unsigned r = inverse[i];
        if (!r)
        {
            h = 0;
            continue;
        }
        unsigned j = t.order[r - 1];
        while (i + h < n && j + h < n && t.bytes[i + h] == t.bytes[j + h])
            h++;
        t.lcp[r] = h;
        if (h)
            h--;
    }
    *s = t;
    return CI_OK;
}

static int compare(const struct ci_suffix *s, unsigned pos,
                   const unsigned char *p, unsigned n)
{
    unsigned i = 0;
    while (i < n && pos + i < s->n && s->bytes[pos + i] == p[i])
        i++;
    if (i == n)
        return 0;
    if (pos + i == s->n)
        return -1;
    return s->bytes[pos + i] < p[i] ? -1 : 1;
}

enum ci_status ci_suffix_range(const struct ci_suffix *s,
                               const unsigned char *p, unsigned n, unsigned *lo,
                               unsigned *hi)
{
    if (!s || !lo || !hi || (!p && n))
        return CI_INVALID;
    if (n > CI_CAP)
        return CI_RANGE;
    unsigned a = 0, b = s->n + 1;
    while (a < b)
    {
        unsigned m = a + (b - a) / 2;
        if (compare(s, s->order[m], p, n) < 0)
            a = m + 1;
        else
            b = m;
    }
    unsigned first = a;
    b = s->n + 1;
    while (a < b)
    {
        unsigned m = a + (b - a) / 2;
        int c = compare(s, s->order[m], p, n);
        if (CI_MUTANT == 8 ? c < 0 : c <= 0)
            a = m + 1;
        else
            b = m;
    }
    *lo = first;
    *hi = a;
    return CI_OK;
}

enum ci_status ci_suffix_lce(const struct ci_suffix *s, unsigned a, unsigned b,
                             unsigned *out)
{
    if (!s || !out)
        return CI_INVALID;
    if (a > s->n || b > s->n)
        return CI_RANGE;
    if (a == b)
    {
        *out = s->n - a;
        return CI_OK;
    }
    unsigned x = 0, y = 0;
    for (unsigned i = 0; i <= s->n; i++)
    {
        if (s->order[i] == a)
            x = i;
        if (s->order[i] == b)
            y = i;
    }
    if (x > y)
    {
        unsigned t = x;
        x = y;
        y = t;
    }
    unsigned v = s->n;
    for (unsigned i = x + 1; i <= y; i++)
        if (s->lcp[i] < v)
            v = s->lcp[i];
    *out = v;
    return CI_OK;
}

enum ci_status ci_suffix_longest(const struct ci_suffix *s, unsigned disjoint,
                                 unsigned *a, unsigned *b, unsigned *length)
{
    if (!s || !a || !b || !length)
        return CI_INVALID;
    if (disjoint > 1)
        return CI_RANGE;
    unsigned best = 0, ba = 0, bb = 0;
    for (unsigned i = 0; i <= s->n; i++)
    {
        unsigned common = s->n;
        for (unsigned j = i + 1; j <= s->n; j++)
        {
            if (s->lcp[j] < common)
                common = s->lcp[j];
            unsigned x = s->order[i], y = s->order[j];
            if (x > y)
            {
                unsigned t = x;
                x = y;
                y = t;
            }
            unsigned n = common;
            if (disjoint && n > y - x)
                n = y - x;
            if (n > best || (n && n == best && (x < ba || (x == ba && y < bb))))
            {
                best = n;
                ba = x;
                bb = y;
            }
        }
    }
    *a = ba;
    *b = bb;
    *length = best;
    return CI_OK;
}
