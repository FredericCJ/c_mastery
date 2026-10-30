/*
 * Reference implementations of the linked and ordered containers: intrusive
 * lists, the generation-checked binary heap and the index-based binary search
 * tree with optional AVL repair.
 *
 * Shared contract (full text in labs/containers_depth/CONTRACTS.md):
 *  - Controls and links are caller-owned, live and at stable addresses;
 *    operations are externally serialized.
 *  - Removing a link detaches it; it never destroys the enclosing payload.
 *  - A refused operation (CD_INVALID, CD_FULL, CD_DUPLICATE, CD_MISSING, ...)
 *    leaves the structure unchanged.
 *
 * The heap section (from the comparison function to the tree) is copied
 * verbatim, apart from the sink function, by assessor/algorithm_heap/generate.py.
 * It must stay self-contained: no file-scope names defined above it.
 *
 * CD_MUTANT selects one deliberately wrong variant for the mutation campaign.
 */
#include "containers.h"

#ifndef CD_MUTANT
#define CD_MUTANT 0
#endif

/* Index-based tree: node indices are 0..TREE_NODES-1, -1 means "none". */
enum
{
    TREE_NODES = 32,
    TREE_NO_NODE = -1
};

static int link_is_detached(const struct cd_link *n)
{
    return !n->owner && !n->next && !n->previous;
}

static void link_detach(struct cd_link *n)
{
    n->next = 0;
    n->previous = 0;
    n->owner = 0;
}

void cd_list_init(struct cd_list *l)
{
    *l = (struct cd_list){0, 0, 0};
}

static void list_push_front(struct cd_list *l, struct cd_link *n)
{
    n->next = l->head;
    if (l->head)
    {
        l->head->previous = n;
    }
    else
    {
        l->tail = n;
    }
    l->head = n;
}

static void list_push_back(struct cd_list *l, struct cd_link *n)
{
    n->previous = l->tail;
    if (l->tail)
    {
        l->tail->next = n;
    }
    else
    {
        l->head = n;
    }
    l->tail = n;
}

/* The link must be detached (all three fields zero).  front selects the end;
 * any other value is CD_INVALID.  The list does not own the link's payload. */
enum cd_status cd_list_push(struct cd_list *l, struct cd_link *n,
                            unsigned front)
{
    if (!l || !n || !link_is_detached(n) || front > 1 || l->count == SIZE_MAX)
    {
        return CD_INVALID;
    }
    n->owner = l;
    if (front)
    {
        list_push_front(l, n);
    }
    else
    {
        list_push_back(l, n);
    }
    ++l->count;
    return CD_OK;
}

/* Repairs both neighbours (or head/tail) and clears the link; the owner field
 * is how a link proves it belongs to this list. */
enum cd_status cd_list_remove(struct cd_list *l, struct cd_link *n)
{
    if (!l || !n || n->owner != l || !l->count)
    {
        return CD_INVALID;
    }
    if (n->previous)
    {
        n->previous->next = n->next;
    }
    else
    {
        l->head = n->next;
    }
    if (n->next)
    {
        n->next->previous = n->previous;
    }
    else
    {
        l->tail = n->previous;
    }
    link_detach(n);
    --l->count;
    return CD_OK;
}

/* Circular list with a sentinel: the sentinel is never a member, so removal
 * needs no head/tail special cases.  `identity` is the owner token that
 * members point at; its count field also holds the member count, which
 * cd_circle_append increments (refusing at SIZE_MAX). */
void cd_circle_init(struct cd_circle *c)
{
    cd_list_init(&c->identity);
    c->sentinel = (struct cd_link){&c->sentinel, &c->sentinel, &c->identity, 0};
}

enum cd_status cd_circle_append(struct cd_circle *c, struct cd_link *n)
{
    if (!c || !n || !link_is_detached(n) || c->identity.count == SIZE_MAX)
    {
        return CD_INVALID;
    }
    struct cd_link *last = c->sentinel.previous;
    n->owner = &c->identity;
    n->next = &c->sentinel;
    n->previous = last;
    last->next = n;
    c->sentinel.previous = n;
    ++c->identity.count;
    return CD_OK;
}

enum cd_status cd_circle_remove(struct cd_circle *c, struct cd_link *n)
{
    if (!c || !n || n == &c->sentinel || n->owner != &c->identity ||
        !c->identity.count)
    {
        return CD_INVALID;
    }
    n->previous->next = n->next;
    n->next->previous = n->previous;
    link_detach(n);
    --c->identity.count;
    return CD_OK;
}

enum cd_status cd_single_append(struct cd_single_list *l, struct cd_single *n)
{
    if (!l || !n || n->linked || n->next || l->count == SIZE_MAX)
    {
        return CD_INVALID;
    }
    if (l->tail)
    {
        l->tail->next = n;
    }
    else
    {
        l->head = n;
    }
    l->tail = n;
    n->linked = 1;
    ++l->count;
    return CD_OK;
}

/* Singly linked removal is a linear search for the predecessor.  CD_MISSING
 * means the node claims membership but is not on this list. */
enum cd_status cd_single_remove(struct cd_single_list *l, struct cd_single *n)
{
    if (!l || !n || !n->linked)
    {
        return CD_INVALID;
    }
    struct cd_single *previous = 0;
    struct cd_single *at = l->head;
    while (at && at != n)
    {
        previous = at;
        at = at->next;
    }
    if (!at)
    {
        return CD_MISSING;
    }
    if (previous)
    {
        previous->next = n->next;
    }
    else
    {
        l->head = n->next;
    }
    if (l->tail == n)
    {
        l->tail = previous;
    }
    n->next = 0;
    n->linked = 0;
    --l->count;
    return CD_OK;
}

/* Heap order: priority (min or max by mode), ties by insertion sequence so
 * equal priorities leave in insertion order.  Every call is one comparison. */
static int before(struct cd_heap *h, struct cd_heap_item a,
                  struct cd_heap_item b)
{
    ++h->comparisons;
    return a.priority != b.priority ? (h->max_heap ? a.priority > b.priority
                                                   : a.priority < b.priority)
                                    : a.sequence < b.sequence;
}

/* Swaps two array positions and keeps the slot -> position map consistent. */
static void swap(struct cd_heap *h, unsigned a, unsigned b)
{
    struct cd_heap_item tmp = h->data[a];
    h->data[a] = h->data[b];
    h->data[b] = tmp;
    if (CD_MUTANT != 4)
    {
        h->index[h->data[a].slot] = a;
        h->index[h->data[b].slot] = b;
    }
}

/* Sifts position `at` toward the root; returns its final position. */
static unsigned rise(struct cd_heap *h, unsigned at)
{
    while (at)
    {
        unsigned parent = (at - 1) / 2;
        if (!before(h, h->data[at], h->data[parent]))
        {
            break;
        }
        swap(h, at, parent);
        at = parent;
    }
    return at;
}

static void sink(struct cd_heap *h, unsigned at)
{ /* Check existence before evaluating2*i+1. */
    while (h->count >= 2 && at <= (h->count - 2) / 2)
    {
        unsigned child = 2 * at + 1;
        if (child + 1 < h->count &&
            before(h, h->data[child + 1], h->data[child]))
        {
            ++child;
        }
        if (!before(h, h->data[child], h->data[at]))
        {
            break;
        }
        swap(h, at, child);
        at = child;
    }
}

/* Handle slot states: 0 free, 1 live (holds an item), 2 retired for good once
 * the slot's generation has reached 3, so a stale handle can never match a
 * reused slot.  The heap holds at most 16 items. */
void cd_heap_init(struct cd_heap *h, unsigned maximum)
{
    h->count = 0;
    h->max_heap = maximum != 0;
    h->sequence = 0;
    h->comparisons = 0;
    for (unsigned i = 0; i < 16; ++i)
    {
        h->index[i] = 0;
        h->generation[i] = 1;
        h->state[i] = 0;
    }
}

/* CD_FULL when 16 items are held, no slot is free or the 32-bit insertion
 * sequence is exhausted; the heap is unchanged in every refusal. */
enum cd_status cd_heap_insert(struct cd_heap *h, int key, unsigned identity,
                              struct cd_heap_handle *out)
{
    if (!h || !out || h->count > 16)
    {
        return CD_INVALID;
    }
    if (h->count == 16 || h->sequence == UINT32_MAX)
    {
        return CD_FULL;
    }
    unsigned slot = 0;
    while (slot < 16 && h->state[slot])
    {
        ++slot;
    }
    if (slot == 16)
    {
        return CD_FULL;
    }
    unsigned at = h->count++;
    h->data[at] = (struct cd_heap_item){key, identity, slot, h->sequence++};
    h->state[slot] = 1;
    h->index[slot] = at;
    rise(h, at);
    *out = (struct cd_heap_handle){h, slot, h->generation[slot]};
    return CD_OK;
}

static int handle_valid(const struct cd_heap *h, struct cd_heap_handle handle)
{
    return h && handle.owner == h && handle.slot < 16 &&
           h->state[handle.slot] == 1 &&
           h->generation[handle.slot] == handle.generation;
}

/* A handle is valid only while its slot is live and its generation matches;
 * the reciprocal check data[at].slot == slot detects a stale index entry. */
enum cd_status cd_heap_get(const struct cd_heap *h,
                           struct cd_heap_handle handle, int *out,
                           unsigned *identity)
{
    if (!out || !identity || !handle_valid(h, handle))
    {
        return CD_INVALID;
    }
    unsigned at = h->index[handle.slot];
    if (at >= h->count || h->data[at].slot != handle.slot)
    {
        return CD_INVALID;
    }
    *out = h->data[at].priority;
    *identity = h->data[at].identity;
    return CD_OK;
}

/* Update-key: rewrites the priority, then repairs in whichever direction is
 * needed (rise first; if it did not move, sink). */
enum cd_status cd_heap_change(struct cd_heap *h, struct cd_heap_handle handle,
                              int value)
{
    if (!handle_valid(h, handle))
    {
        return CD_INVALID;
    }
    unsigned at = h->index[handle.slot];
    if (at >= h->count || h->data[at].slot != handle.slot)
    {
        return CD_INVALID;
    }
    h->data[at].priority = value;
    at = rise(h, at);
    sink(h, at);
    return CD_OK;
}

/* Removes the root.  Its slot's generation advances so the old handle goes
 * stale; after generation 3 the slot is retired instead of reused. */
enum cd_status cd_heap_take(struct cd_heap *h, int *out, unsigned *identity)
{
    if (!h || !out || !identity || h->count > 16)
    {
        return CD_INVALID;
    }
    if (!h->count)
    {
        return CD_EMPTY;
    }
    struct cd_heap_item item = h->data[0];
    --h->count;
    if (h->count)
    {
        h->data[0] = h->data[h->count];
        h->index[h->data[0].slot] = 0;
        sink(h, 0);
    }
    unsigned slot = item.slot;
    if (h->generation[slot] == 3)
    {
        h->state[slot] = 2;
    }
    else
    {
        ++h->generation[slot];
        h->state[slot] = 0;
    }
    *out = item.priority;
    *identity = item.identity;
    return CD_OK;
}

/* Bottom-up build: place all items, then sink every internal node from the
 * last parent down to the root. */
enum cd_status cd_heap_build(struct cd_heap *h, const int *input, size_t n,
                             unsigned maximum)
{
    if (!h || (!input && n) || n > 16)
    {
        return CD_INVALID;
    }
    cd_heap_init(h, maximum);
    h->count = (unsigned)n;
    h->sequence = (uint32_t)n;
    for (unsigned i = 0; i < n; ++i)
    {
        h->data[i] = (struct cd_heap_item){input[i], i, i, i};
        h->index[i] = i;
        h->state[i] = 1;
    }
    for (unsigned i = h->count / 2; i > 0; --i)
    {
        sink(h, i - 1);
    }
    return CD_OK;
}

void cd_tree_init(struct cd_tree *t, unsigned balanced, unsigned multiplicity)
{
    t->root = TREE_NO_NODE;
    t->count = 0;
    t->rotations = 0;
    t->repair_steps = 0;
    t->balanced = balanced != 0;
    t->multiplicity = multiplicity != 0;
    for (unsigned i = 0; i < TREE_NODES; ++i)
    {
        t->nodes[i] = (struct cd_tree_node){0, -1, -1, -1, 0, 0, 0, 1};
    }
}

/* Returns the node index holding key, or -1. */
int cd_tree_find(const struct cd_tree *t, int key)
{
    if (!t)
    {
        return TREE_NO_NODE;
    }
    int at = t->root;
    while (at >= 0)
    {
        if (t->nodes[at].key == key)
        {
            return at;
        }
        at = key < t->nodes[at].key ? t->nodes[at].left : t->nodes[at].right;
    }
    return TREE_NO_NODE;
}

static int height(const struct cd_tree *t, int at)
{
    return at < 0 ? 0 : t->nodes[at].height;
}

static void update(struct cd_tree *t, int at)
{
    int left = height(t, t->nodes[at].left);
    int right = height(t, t->nodes[at].right);
    t->nodes[at].height = 1 + (left > right ? left : right);
}

/* Makes `next` take `old`'s place under old's parent (or as root).  Does not
 * touch old's own links. */
static void replace(struct cd_tree *t, int old, int next)
{
    int parent = t->nodes[old].parent;
    if (parent < 0)
    {
        t->root = next;
    }
    else if (t->nodes[parent].left == old)
    {
        t->nodes[parent].left = next;
    }
    else
    {
        t->nodes[parent].right = next;
    }
    if (next >= 0)
    {
        t->nodes[next].parent = parent;
    }
}

/* Rotations return the new subtree root.  Heights are recomputed bottom-up:
 * the demoted node first, then its replacement. */
static int left_rotate(struct cd_tree *t, int at)
{
    ++t->rotations;
    int next = t->nodes[at].right;
    int inner = t->nodes[next].left;
    replace(t, at, next);
    t->nodes[next].left = at;
    t->nodes[at].parent = next;
    t->nodes[at].right = inner;
    if (inner >= 0)
    {
        t->nodes[inner].parent = at;
    }
    if (CD_MUTANT != 5)
    {
        update(t, at);
    }
    update(t, next);
    return next;
}

static int right_rotate(struct cd_tree *t, int at)
{
    ++t->rotations;
    int next = t->nodes[at].left;
    int inner = t->nodes[next].right;
    replace(t, at, next);
    t->nodes[next].right = at;
    t->nodes[at].parent = next;
    t->nodes[at].left = inner;
    if (inner >= 0)
    {
        t->nodes[inner].parent = at;
    }
    update(t, at);
    update(t, next);
    return next;
}

/* Left-heavy node: a left-right shape needs the left child rotated first. */
static int rebalance_left_heavy(struct cd_tree *t, int at)
{
    int left = t->nodes[at].left;
    if (height(t, t->nodes[left].left) < height(t, t->nodes[left].right))
    {
        left_rotate(t, left);
    }
    return right_rotate(t, at);
}

static int rebalance_right_heavy(struct cd_tree *t, int at)
{
    int right = t->nodes[at].right;
    if (height(t, t->nodes[right].right) < height(t, t->nodes[right].left))
    {
        right_rotate(t, right);
    }
    return left_rotate(t, at);
}

/* Walks from `at` to the root refreshing heights and, in balanced mode,
 * restoring |balance factor| <= 1.  Counts one repair step per ancestor. */
static void repair(struct cd_tree *t, int at)
{
    while (at >= 0)
    {
        ++t->repair_steps;
        update(t, at);
        int difference =
            height(t, t->nodes[at].left) - height(t, t->nodes[at].right);
        if (t->balanced && difference > 1)
        {
            at = rebalance_left_heavy(t, at);
        }
        else if (t->balanced && difference < -1)
        {
            at = rebalance_right_heavy(t, at);
        }
        at = t->nodes[at].parent;
    }
}

/* Returns the first slot that is not live and whose generation can still
 * advance (a slot at UINT32_MAX is retired), or TREE_NODES if none. */
static unsigned tree_free_slot(const struct cd_tree *t)
{
    unsigned slot = 0;
    while (slot < TREE_NODES &&
           (t->nodes[slot].live || t->nodes[slot].generation == UINT32_MAX))
    {
        ++slot;
    }
    return slot;
}

/* Hangs the new node under the leaf where an ordinary search for key ends.
 * Returns that leaf's index (the first ancestor to repair), or -1 for the
 * root. */
static int tree_attach_leaf(struct cd_tree *t, unsigned slot, int key)
{
    int parent = TREE_NO_NODE;
    int at = t->root;
    while (at >= 0)
    {
        parent = at;
        at = key < t->nodes[at].key ? t->nodes[at].left : t->nodes[at].right;
    }
    uint32_t generation = t->nodes[slot].generation;
    t->nodes[slot] =
        (struct cd_tree_node){key, -1, -1, parent, 1, 1, 1, generation};
    if (parent < 0)
    {
        t->root = (int)slot;
    }
    else if (key < t->nodes[parent].key)
    {
        t->nodes[parent].left = (int)slot;
    }
    else
    {
        t->nodes[parent].right = (int)slot;
    }
    return parent;
}

/* A duplicate key is CD_DUPLICATE in set mode and a multiplicity increment in
 * multiset mode.  `fail` simulates node acquisition failure (CD_NOMEM); it is
 * checked before any link or height changes.  *out is the node index. */
enum cd_status cd_tree_insert(struct cd_tree *t, int key, unsigned fail,
                              int *out)
{
    if (!t || !out)
    {
        return CD_INVALID;
    }
    int found = cd_tree_find(t, key);
    if (found >= 0)
    {
        if (!t->multiplicity)
        {
            return CD_DUPLICATE;
        }
        if (t->nodes[found].multiplicity == UINT32_MAX)
        {
            return CD_FULL;
        }
        ++t->nodes[found].multiplicity;
        *out = found;
        return CD_OK;
    }
    if (fail)
    {
        return CD_NOMEM;
    }
    unsigned slot = tree_free_slot(t);
    if (slot == TREE_NODES)
    {
        return CD_FULL;
    }
    int parent = tree_attach_leaf(t, slot, key);
    ++t->count;
    repair(t, parent);
    *out = (int)slot;
    return CD_OK;
}

static int tree_leftmost(const struct cd_tree *t, int at)
{
    while (t->nodes[at].left >= 0)
    {
        at = t->nodes[at].left;
    }
    return at;
}

/* Unlinks the node `at`, which has two children, by transplanting its
 * in-order successor into its place.  The successor keeps its own slot, so
 * its identity (and anything keyed to the slot) survives; only the victim's
 * slot is invalidated by the caller.  Returns the first node to repair. */
static int tree_unlink_two_children(struct cd_tree *t, int at)
{
    const struct cd_tree_node *n = &t->nodes[at];
    int successor = tree_leftmost(t, n->right);
    int start = successor;
    if (t->nodes[successor].parent != at)
    {
        start = t->nodes[successor].parent;
        replace(t, successor, t->nodes[successor].right);
        t->nodes[successor].right = n->right;
        t->nodes[n->right].parent = successor;
    }
    replace(t, at, successor);
    t->nodes[successor].left = n->left;
    t->nodes[n->left].parent = successor;
    update(t, successor);
    return start;
}

/* Removes one occurrence of key.  A node with multiplicity above one only has
 * its multiplicity decremented; the node is unlinked when its last occurrence
 * (multiplicity one) is removed.  CD_MISSING if key is absent. */
enum cd_status cd_tree_delete(struct cd_tree *t, int key)
{
    if (!t)
    {
        return CD_INVALID;
    }
    int at = cd_tree_find(t, key);
    if (at < 0)
    {
        return CD_MISSING;
    }
    struct cd_tree_node *n = &t->nodes[at];
    if (n->multiplicity > 1)
    {
        --n->multiplicity;
        return CD_OK;
    }
    int start = n->parent;
    if (n->left < 0)
    {
        replace(t, at, n->right);
    }
    else if (n->right < 0)
    {
        replace(t, at, n->left);
    }
    else
    {
        start = tree_unlink_two_children(t, at);
    }
    n->live = 0;
    n->left = TREE_NO_NODE;
    n->right = TREE_NO_NODE;
    n->parent = TREE_NO_NODE;
    n->height = 0;
    n->multiplicity = 0;
    if (n->generation < UINT32_MAX)
    {
        ++n->generation;
    }
    --t->count;
    repair(t, start);
    return CD_OK;
}

/* Appends one node's key `multiplicity` times.  CD_FULL when out is full; the
 * keys written so far stay and are reported through *written. */
static enum cd_status emit_key(const struct cd_tree_node *node, int *out,
                               size_t capacity, size_t *written)
{
    for (unsigned i = 0; i < node->multiplicity; ++i)
    {
        if (*written == capacity)
        {
            return CD_FULL;
        }
        out[(*written)++] = node->key;
    }
    return CD_OK;
}

/* In-order walk with a caller-supplied stack of node indices (at most one per
 * tree level, so TREE_NODES entries always suffice).  On CD_FULL (stack or
 * output too small) *used is the count already written. */
enum cd_status cd_tree_walk(const struct cd_tree *t, unsigned *stack,
                            size_t stack_cap, int *out, size_t capacity,
                            size_t *used)
{
    if (!t || !used || (!stack && stack_cap) || (!out && capacity) ||
        stack_cap > TREE_NODES)
    {
        return CD_INVALID;
    }
    size_t top = 0;
    size_t written = 0;
    int at = t->root;
    while (at >= 0 || top)
    {
        while (at >= 0)
        {
            if (top == stack_cap)
            {
                *used = written;
                return CD_FULL;
            }
            stack[top++] = (unsigned)at;
            at = t->nodes[at].left;
        }
        at = (int)stack[--top];
        if (emit_key(&t->nodes[at], out, capacity, &written) != CD_OK)
        {
            *used = written;
            return CD_FULL;
        }
        at = t->nodes[at].right;
    }
    *used = written;
    return CD_OK;
}

/* In-order walk with constant auxiliary storage: the successor of a node is
 * the leftmost node of its right subtree, else the nearest ancestor reached
 * from a left child.  Relies on the parent links being consistent. */
enum cd_status cd_tree_parent_walk(const struct cd_tree *t, int *out,
                                   size_t capacity, size_t *used)
{
    if (!t || !used || (!out && capacity))
    {
        return CD_INVALID;
    }
    size_t written = 0;
    int at = t->root;
    if (at >= 0)
    {
        at = tree_leftmost(t, at);
    }
    while (at >= 0)
    {
        if (emit_key(&t->nodes[at], out, capacity, &written) != CD_OK)
        {
            *used = written;
            return CD_FULL;
        }
        if (t->nodes[at].right >= 0)
        {
            at = tree_leftmost(t, t->nodes[at].right);
        }
        else
        {
            int parent = t->nodes[at].parent;
            while (parent >= 0 && t->nodes[parent].right == at)
            {
                at = parent;
                parent = t->nodes[at].parent;
            }
            at = parent;
        }
    }
    *used = written;
    return CD_OK;
}
