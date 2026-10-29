#include "index.h"
#ifndef CI_MUTANT
#define CI_MUTANT 0
#endif
static int height(const struct ci_intervals *t, int p)
{
    return p < 0 ? 0 : t->node[p].height;
}

static int before(struct ci_interval a, struct ci_interval b)
{
    return a.low < b.low || (a.low == b.low && a.id < b.id);
}

static void fix(struct ci_intervals *t, int p)
{
    struct ci_interval_node *n = &t->node[p];
    int a = height(t, n->left), b = height(t, n->right);
    n->height = 1 + (a > b ? a : b);
    n->maximum = n->value.high;
    if (CI_MUTANT != 5)
    {
        if (n->left >= 0 && t->node[n->left].maximum > n->maximum)
            n->maximum = t->node[n->left].maximum;
        if (n->right >= 0 && t->node[n->right].maximum > n->maximum)
            n->maximum = t->node[n->right].maximum;
    }
}

static int left(struct ci_intervals *t, int p)
{
    int q = t->node[p].right;
    t->node[p].right = t->node[q].left;
    t->node[q].left = p;
    fix(t, p);
    fix(t, q);
    return q;
}

static int right(struct ci_intervals *t, int p)
{
    int q = t->node[p].left;
    t->node[p].left = t->node[q].right;
    t->node[q].right = p;
    fix(t, p);
    fix(t, q);
    return q;
}

static int balance(struct ci_intervals *t, int p)
{
    fix(t, p);
    int b = height(t, t->node[p].left) - height(t, t->node[p].right);
    if (b > 1)
    {
        int q = t->node[p].left;
        if (height(t, t->node[q].left) < height(t, t->node[q].right))
            t->node[p].left = left(t, q);
        return right(t, p);
    }
    if (b < -1)
    {
        int q = t->node[p].right;
        if (height(t, t->node[q].right) < height(t, t->node[q].left))
            t->node[p].right = right(t, q);
        return left(t, p);
    }
    return p;
}

enum ci_status ci_intervals_init(struct ci_intervals *t, unsigned closed)
{
    if (!t)
        return CI_INVALID;
    if (closed > 1)
        return CI_RANGE;
    t->root = -1;
    t->free_head = 0;
    t->count = 0;
    t->closed = closed;
    for (unsigned i = 0; i < CI_CAP; i++)
    {
        struct ci_interval_node *n = &t->node[i];
        n->live = 0;
        n->left = n->right = -1;
        n->height = 0;
        n->maximum = 0;
        n->value.low = n->value.high = 0;
        n->value.id = 0;
        n->next = i + 1 < CI_CAP ? (int)i + 1 : -1;
    }
    return CI_OK;
}

static int insert(struct ci_intervals *t, int p, int v)
{
    if (p < 0)
        return v;
    if (before(t->node[v].value, t->node[p].value))
        t->node[p].left = insert(t, t->node[p].left, v);
    else
        t->node[p].right = insert(t, t->node[p].right, v);
    return balance(t, p);
}

static int locate(const struct ci_intervals *t, struct ci_interval v)
{
    int p = t->root;
    while (p >= 0)
    {
        struct ci_interval a = t->node[p].value;
        if (a.low == v.low && a.id == v.id)
            return p;
        p = before(v, a) ? t->node[p].left : t->node[p].right;
    }
    return -1;
}

enum ci_status ci_intervals_insert(struct ci_intervals *t, struct ci_interval v)
{
    if (!t)
        return CI_INVALID;
    if (v.low > v.high || (!t->closed && v.low == v.high))
        return CI_RANGE;
    if (locate(t, v) >= 0)
        return CI_DUPLICATE;
    if (t->free_head < 0)
        return CI_FULL;
    int p = t->free_head;
    struct ci_interval_node *n = &t->node[p];
    t->free_head = n->next;
    n->value = v;
    n->left = n->right = -1;
    n->height = 1;
    n->maximum = v.high;
    n->live = 1;
    n->next = -1;
    t->root = insert(t, t->root, p);
    t->count++;
    return CI_OK;
}

static int remove_node(struct ci_intervals *t, int p, struct ci_interval v)
{
    struct ci_interval_node *n = &t->node[p];
    if (before(v, n->value))
        n->left = remove_node(t, n->left, v);
    else if (before(n->value, v))
        n->right = remove_node(t, n->right, v);
    else
    {
        if (n->left < 0 || n->right < 0)
        {
            int child = n->left < 0 ? n->right : n->left;
            n->live = 0;
            n->next = t->free_head;
            t->free_head = p;
            return child;
        }
        int q = n->right;
        while (t->node[q].left >= 0)
            q = t->node[q].left;
        n->value = t->node[q].value;
        n->right = remove_node(t, n->right, n->value);
    }
    return balance(t, p);
}

enum ci_status ci_intervals_remove(struct ci_intervals *t, int32_t low,
                                   uint32_t id)
{
    if (!t)
        return CI_INVALID;
    struct ci_interval v = {low, low, id};
    if (locate(t, v) < 0)
        return CI_MISSING;
    t->root = remove_node(t, t->root, v);
    t->count--;
    return CI_OK;
}

static int overlaps(const struct ci_intervals *t, struct ci_interval x,
                    int32_t lo, int32_t hi)
{
    return (t->closed || CI_MUTANT == 6) ? x.low <= hi && lo <= x.high
                                         : x.low < hi && lo < x.high;
}

static int viable(const struct ci_intervals *t, int p, int32_t lo)
{
    return p >= 0 &&
           (t->closed ? t->node[p].maximum >= lo : t->node[p].maximum > lo);
}

enum ci_status ci_intervals_find(const struct ci_intervals *t, int32_t lo,
                                 int32_t hi, struct ci_interval *out)
{
    if (!t || !out)
        return CI_INVALID;
    if (lo > hi || (!t->closed && lo == hi))
        return CI_RANGE;
    int p = t->root;
    while (p >= 0)
    {
        if (overlaps(t, t->node[p].value, lo, hi))
        {
            *out = t->node[p].value;
            return CI_OK;
        }
        p = viable(t, t->node[p].left, lo) ? t->node[p].left : t->node[p].right;
    }
    return CI_MISSING;
}

static void gather(const struct ci_intervals *t, int p, int32_t lo, int32_t hi,
                   struct ci_interval *out, unsigned *n)
{
    if (!viable(t, p, lo))
        return;
    gather(t, t->node[p].left, lo, hi, out, n);
    struct ci_interval x = t->node[p].value;
    if (overlaps(t, x, lo, hi))
        out[(*n)++] = x;
    if (t->closed ? x.low <= hi : x.low < hi)
        gather(t, t->node[p].right, lo, hi, out, n);
}

enum ci_status ci_intervals_all(const struct ci_intervals *t, int32_t lo,
                                int32_t hi, struct ci_interval *out,
                                unsigned cap, unsigned *count)
{
    if (!t || !count || (!out && cap))
        return CI_INVALID;
    if (lo > hi || (!t->closed && lo == hi))
        return CI_RANGE;
    struct ci_interval temp[CI_CAP];
    unsigned n = 0;
    gather(t, t->root, lo, hi, temp, &n);
    if (n > cap)
        return CI_FULL;
    for (unsigned i = 0; i < n; i++)
        out[i] = temp[i];
    *count = n;
    return CI_OK;
}
