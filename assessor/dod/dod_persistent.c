#include "dod_persistent.h"
#ifndef D_PERSIST_MUTANT
#define D_PERSIST_MUTANT 0
#endif
int d_zip_open(struct d_tree *t, unsigned root, struct d_zipper *out)
{
    if (!t || !out || root > t->used)
    {
        return D_INVALID;
    }
    *out = (struct d_zipper){t, root, root, 0, {0}, {0}};
    return D_OK;
}

int d_zip_down(struct d_zipper *z, unsigned direction)
{
    if (!z || !z->arena || direction > 1 || z->depth >= 3)
    {
        return D_INVALID;
    }
    z->crumb[z->depth] = z->focus;
    z->direction[z->depth++] = direction;
    if (z->focus)
    {
        z->focus = direction ? z->arena->nodes[z->focus].right
                             : z->arena->nodes[z->focus].left;
    }
    return D_OK;
}

int d_zip_up(struct d_zipper *z)
{
    if (!z || !z->arena || !z->depth)
    {
        return D_INVALID;
    }
    z->focus = z->crumb[--z->depth];
    return D_OK;
}

static unsigned tree_new(struct d_tree *t, struct d_tree_node node)
{
    ++t->allocations;
    if (t->used == 64 || (t->fail_after && t->allocations == t->fail_after))
    {
        return 0;
    }
    t->nodes[++t->used] = node;
    return t->used;
}

int d_zip_write(const struct d_zipper *z, int value, unsigned *out)
{
    if (!z || !z->arena || !out || z->depth != 3)
    {
        return D_INVALID;
    }
    struct d_tree *t = z->arena;
    unsigned before = t->used;
    t->allocations = 0;
    unsigned child = tree_new(t, (struct d_tree_node){0, 0, value});
    for (unsigned depth = 3; child && depth;)
    {
        --depth;
        struct d_tree_node n = {0, 0, 0};
        if (z->crumb[depth])
        {
            n = t->nodes[z->crumb[depth]];
        }
        if (z->direction[depth])
        {
            n.right = child;
        }
        else
        {
            n.left = child;
        }
        if (D_PERSIST_MUTANT == 1 && z->crumb[depth])
        {
            t->nodes[z->crumb[depth]] = n;
        }
        child = tree_new(t, n);
    }
    if (!child)
    {
        if (D_PERSIST_MUTANT != 2)
        {
            t->used = before;
        }
        return D_FAILED;
    }
    *out = child;
    return D_OK;
}

static void census(const struct d_tree *t, unsigned node, unsigned *refs)
{
    if (!node)
    {
        return;
    }
    if (refs[node]++)
    {
        return;
    }
    census(t, t->nodes[node].left, refs);
    census(t, t->nodes[node].right, refs);
}

int d_tree_census(const struct d_tree *t, const unsigned *roots, size_t n,
                  unsigned *live, unsigned *shared)
{
    if (!t || !live || !shared || (!roots && n) || n > 16)
    {
        return D_INVALID;
    }
    for (size_t i = 0; i < n; ++i)
    {
        if (roots[i] > t->used)
        {
            return D_INVALID;
        }
    }
    unsigned refs[65] = {0}, a = 0, b = 0;
    for (size_t i = 0; i < n; ++i)
    {
        census(t, roots[i], refs);
    }
    for (unsigned i = 1; i <= t->used; ++i)
    {
        a += refs[i] != 0;
        b += refs[i] > 1;
    }
    *live = a;
    *shared = b;
    return D_OK;
}

static unsigned rank(unsigned mask, unsigned slot)
{
    unsigned n = 0;
    for (unsigned i = 0; i < slot; ++i)
    {
        n += (mask >> i) & 1u;
    }
    return n;
}

static unsigned map_new(struct d_hamt *h, struct d_hamt_node n)
{
    ++h->allocations;
    if (h->used == 64 || (h->fail_after && h->allocations == h->fail_after))
    {
        return 0;
    }
    h->nodes[++h->used] = n;
    return h->used;
}

static int remove_key(struct d_hamt *h, unsigned root, unsigned key,
                      unsigned depth, unsigned *out)
{
    struct d_hamt_node n = h->nodes[root];
    if (depth == 2)
    {
        unsigned pos = 0;
        while (pos < n.count && n.keys[pos] != key)
        {
            ++pos;
        }
        if (pos == n.count)
        {
            return D_STALE;
        }
        for (unsigned i = pos + 1; i < n.count; ++i)
        {
            n.keys[i - 1] = n.keys[i];
            n.values[i - 1] = n.values[i];
        }
        --n.count;
    }
    else
    {
        unsigned slot = (key >> (2 * depth)) & 3;
        if (!(n.bitmap & (1u << slot)))
        {
            return D_STALE;
        }
        unsigned at = rank(n.bitmap, slot), child = 0;
        int r = remove_key(h, n.children[at], key, depth + 1, &child);
        if (r)
        {
            return r;
        }
        if (child)
        {
            n.children[at] = child;
        }
        else
        {
            for (unsigned i = at + 1; i < n.count; ++i)
            {
                n.children[i - 1] = n.children[i];
            }
            --n.count;
            if (D_PERSIST_MUTANT != 4)
            {
                n.bitmap &= ~(1u << slot);
            }
        }
    }
    if (!n.count)
    {
        *out = 0;
        return D_OK;
    }
    if (D_PERSIST_MUTANT == 3)
    {
        h->nodes[root] = n;
    }
    unsigned result = map_new(h, n);
    if (!result)
    {
        return D_FAILED;
    }
    *out = result;
    return D_OK;
}

int d_hamt_delete(struct d_hamt *h, unsigned root, unsigned key, unsigned *out)
{
    if (!h || !out || root > h->used || key >= 32)
    {
        return D_INVALID;
    }
    if (!root)
    {
        return D_STALE;
    }
    unsigned before = h->used, next = 0;
    h->allocations = 0;
    int r = remove_key(h, root, key, 0, &next);
    if (r)
    {
        h->used = before;
        return r;
    }
    *out = next;
    return D_OK;
}

void d_rc_init(struct d_rc_tree *t, struct d_text_ops ops)
{
    *t = (struct d_rc_tree){0};
    t->ops = ops;
}

int d_rc_retain(struct d_rc_tree *t, unsigned root)
{
    if (!t || root > 64 || (root && !t->nodes[root].refs))
    {
        return D_INVALID;
    }
    if (!root)
    {
        return D_OK;
    }
    if (t->nodes[root].refs == 32)
    {
        return D_FULL;
    }
    ++t->nodes[root].refs;
    return D_OK;
}

int d_rc_release(struct d_rc_tree *t, unsigned root)
{
    if (!t || root > 64 || (root && !t->nodes[root].refs))
    {
        return D_INVALID;
    }
    if (!root)
    {
        return D_OK;
    }
    struct d_rc_node *n = &t->nodes[root];
    if (--n->refs)
    {
        return D_OK;
    }
    unsigned left = n->left, right = n->right;
    unsigned char *text = n->text;
    *n = (struct d_rc_node){0};
    --t->live;
    ++t->reclaimed;
    if (text)
    {
        t->ops.drop(t->ops.context, text);
    }
    if (left)
    {
        (void)d_rc_release(t, left);
    }
    if (right && D_PERSIST_MUTANT != 5)
    {
        (void)d_rc_release(t, right);
    }
    return D_OK;
}

static unsigned rc_new(struct d_rc_tree *t, struct d_rc_node node)
{
    ++t->attempts;
    if (t->fail_at && t->attempts == t->fail_at)
    {
        return 0;
    }
    for (unsigned i = 1; i <= 64; ++i)
    {
        if (!t->nodes[i].refs)
        {
            node.refs = 1;
            t->nodes[i] = node;
            ++t->live;
            return i;
        }
    }
    return 0;
}

static unsigned rc_set(struct d_rc_tree *t, unsigned root, unsigned depth,
                       unsigned index, const unsigned char *p, size_t length)
{
    if (depth == 3)
    {
        unsigned char *copy = NULL;
        if (!t->ops.clone(t->ops.context, p, length, &copy))
        {
            return 0;
        }
        unsigned node = rc_new(t, (struct d_rc_node){0, 0, 0, copy, length});
        if (!node && copy)
        {
            t->ops.drop(t->ops.context, copy);
        }
        return node;
    }
    struct d_rc_node old = {0};
    if (root)
    {
        old = t->nodes[root];
    }
    unsigned right = (index >> (2 - depth)) & 1;
    unsigned child =
        rc_set(t, right ? old.right : old.left, depth + 1, index, p, length);
    if (!child)
    {
        return 0;
    }
    unsigned sibling = right ? old.left : old.right;
    if (d_rc_retain(t, sibling) != D_OK)
    {
        (void)d_rc_release(t, child);
        return 0;
    }
    struct d_rc_node n = {0};
    n.left = right ? sibling : child;
    n.right = right ? child : sibling;
    unsigned result = rc_new(t, n);
    if (!result)
    {
        (void)d_rc_release(t, child);
        if (D_PERSIST_MUTANT != 6)
        {
            (void)d_rc_release(t, sibling);
        }
    }
    return result;
}

int d_rc_update(struct d_rc_tree *t, unsigned root, unsigned index,
                const unsigned char *p, size_t n, unsigned *out)
{
    if (!t || !out || !t->ops.clone || !t->ops.drop || root > 64 ||
        (root && !t->nodes[root].refs) || index >= 8 || (!p && n) || n > 32)
    {
        return D_INVALID;
    }
    t->attempts = 0;
    unsigned result = rc_set(t, root, 0, index, p, n);
    if (!result)
    {
        return D_FAILED;
    }
    *out = result;
    return D_OK;
}

int d_rc_get(const struct d_rc_tree *t, unsigned root, unsigned index,
             const unsigned char **out, size_t *n)
{
    if (!t || !out || !n || root > 64 || (root && !t->nodes[root].refs) ||
        index >= 8)
    {
        return D_INVALID;
    }
    for (unsigned depth = 0; root && depth < 3; ++depth)
    {
        root = (index >> (2 - depth)) & 1 ? t->nodes[root].right
                                          : t->nodes[root].left;
    }
    *out = root ? t->nodes[root].text : NULL;
    *n = root ? t->nodes[root].length : 0;
    return D_OK;
}

int d_rc_retire(struct d_rc_tree *t, unsigned root, struct d_rc_retire *work)
{
    if (!t || !work || work->owner || work->count || root > 64 ||
        (root && !t->nodes[root].refs))
    {
        return D_INVALID;
    }
    if (root)
    {
        work->owner = t;
        work->pending[0] = root;
        work->count = 1;
    }
    return D_OK;
}

int d_rc_drain(struct d_rc_retire *work, unsigned budget, unsigned *done)
{
    if (!work || !done || (!work->owner && work->count))
    {
        return D_INVALID;
    }
    struct d_rc_tree *t = work->owner;
    while (budget && work->count)
    {
        --budget;
        unsigned root = work->pending[--work->count];
        struct d_rc_node *n = &t->nodes[root];
        if (--n->refs)
        {
            continue;
        }
        unsigned left = n->left, right = n->right;
        unsigned char *text = n->text;
        *n = (struct d_rc_node){0};
        --t->live;
        ++t->reclaimed;
        if (text)
        {
            t->ops.drop(t->ops.context, text);
        }
        if (left)
        {
            work->pending[work->count++] = left;
        }
        if (right)
        {
            work->pending[work->count++] = right;
        }
    }
    *done = work->count == 0;
    if (*done)
    {
        work->owner = NULL;
    }
    return D_OK;
}
