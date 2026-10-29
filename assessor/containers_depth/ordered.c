#include "containers.h"
#ifndef CD_MUTANT
#define CD_MUTANT 0
#endif
void cd_list_init(struct cd_list *l)
{
    *l = (struct cd_list){0, 0, 0};
}

enum cd_status cd_list_push(struct cd_list *l, struct cd_link *n,
                            unsigned front)
{
    if (!l || !n || n->owner || n->next || n->previous || front > 1 ||
        l->count == SIZE_MAX)
        return CD_INVALID;
    n->owner = l;
    if (front)
    {
        n->next = l->head;
        if (l->head)
            l->head->previous = n;
        else
            l->tail = n;
        l->head = n;
    }
    else
    {
        n->previous = l->tail;
        if (l->tail)
            l->tail->next = n;
        else
            l->head = n;
        l->tail = n;
    }
    ++l->count;
    return CD_OK;
}

enum cd_status cd_list_remove(struct cd_list *l, struct cd_link *n)
{
    if (!l || !n || n->owner != l || !l->count)
        return CD_INVALID;
    if (n->previous)
        n->previous->next = n->next;
    else
        l->head = n->next;
    if (n->next)
        n->next->previous = n->previous;
    else
        l->tail = n->previous;
    n->next = n->previous = 0;
    n->owner = 0;
    --l->count;
    return CD_OK;
}

void cd_circle_init(struct cd_circle *c)
{
    cd_list_init(&c->identity);
    c->sentinel = (struct cd_link){&c->sentinel, &c->sentinel, &c->identity, 0};
}

enum cd_status cd_circle_append(struct cd_circle *c, struct cd_link *n)
{
    if (!c || !n || n->owner || n->next || n->previous ||
        c->identity.count == SIZE_MAX)
        return CD_INVALID;
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
        return CD_INVALID;
    n->previous->next = n->next;
    n->next->previous = n->previous;
    n->next = n->previous = 0;
    n->owner = 0;
    --c->identity.count;
    return CD_OK;
}

enum cd_status cd_single_append(struct cd_single_list *l, struct cd_single *n)
{
    if (!l || !n || n->linked || n->next || l->count == SIZE_MAX)
        return CD_INVALID;
    if (l->tail)
        l->tail->next = n;
    else
        l->head = n;
    l->tail = n;
    n->linked = 1;
    ++l->count;
    return CD_OK;
}

enum cd_status cd_single_remove(struct cd_single_list *l, struct cd_single *n)
{
    if (!l || !n || !n->linked)
        return CD_INVALID;
    struct cd_single *previous = 0, *at = l->head;
    while (at && at != n)
    {
        previous = at;
        at = at->next;
    }
    if (!at)
        return CD_MISSING;
    if (previous)
        previous->next = n->next;
    else
        l->head = n->next;
    if (l->tail == n)
        l->tail = previous;
    n->next = 0;
    n->linked = 0;
    --l->count;
    return CD_OK;
}

static int before(struct cd_heap *h, struct cd_heap_item a,
                  struct cd_heap_item b)
{
    ++h->comparisons;
    return a.priority != b.priority ? (h->max_heap ? a.priority > b.priority
                                                   : a.priority < b.priority)
                                    : a.sequence < b.sequence;
}

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

static unsigned rise(struct cd_heap *h, unsigned at)
{
    while (at)
    {
        unsigned parent = (at - 1) / 2;
        if (!before(h, h->data[at], h->data[parent]))
            break;
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
            ++child;
        if (!before(h, h->data[child], h->data[at]))
            break;
        swap(h, at, child);
        at = child;
    }
}

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

enum cd_status cd_heap_insert(struct cd_heap *h, int key, unsigned identity,
                              struct cd_heap_handle *out)
{
    if (!h || !out || h->count > 16)
        return CD_INVALID;
    if (h->count == 16 || h->sequence == UINT32_MAX)
        return CD_FULL;
    unsigned slot = 0;
    while (slot < 16 && h->state[slot])
        ++slot;
    if (slot == 16)
        return CD_FULL;
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

enum cd_status cd_heap_get(const struct cd_heap *h,
                           struct cd_heap_handle handle, int *out,
                           unsigned *identity)
{
    if (!out || !identity || !handle_valid(h, handle))
        return CD_INVALID;
    unsigned at = h->index[handle.slot];
    if (at >= h->count || h->data[at].slot != handle.slot)
        return CD_INVALID;
    *out = h->data[at].priority;
    *identity = h->data[at].identity;
    return CD_OK;
}

enum cd_status cd_heap_change(struct cd_heap *h, struct cd_heap_handle handle,
                              int value)
{
    if (!handle_valid(h, handle))
        return CD_INVALID;
    unsigned at = h->index[handle.slot];
    if (at >= h->count || h->data[at].slot != handle.slot)
        return CD_INVALID;
    h->data[at].priority = value;
    at = rise(h, at);
    sink(h, at);
    return CD_OK;
}

enum cd_status cd_heap_take(struct cd_heap *h, int *out, unsigned *identity)
{
    if (!h || !out || !identity || h->count > 16)
        return CD_INVALID;
    if (!h->count)
        return CD_EMPTY;
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
        h->state[slot] = 2;
    else
    {
        ++h->generation[slot];
        h->state[slot] = 0;
    }
    *out = item.priority;
    *identity = item.identity;
    return CD_OK;
}

enum cd_status cd_heap_build(struct cd_heap *h, const int *input, size_t n,
                             unsigned maximum)
{
    if (!h || (!input && n) || n > 16)
        return CD_INVALID;
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
        sink(h, i - 1);
    return CD_OK;
}

void cd_tree_init(struct cd_tree *t, unsigned balanced, unsigned multiplicity)
{
    t->root = -1;
    t->count = 0;
    t->rotations = t->repair_steps = 0;
    t->balanced = balanced != 0;
    t->multiplicity = multiplicity != 0;
    for (unsigned i = 0; i < 32; ++i)
        t->nodes[i] = (struct cd_tree_node){0, -1, -1, -1, 0, 0, 0, 1};
}

int cd_tree_find(const struct cd_tree *t, int key)
{
    if (!t)
        return -1;
    int at = t->root;
    while (at >= 0)
    {
        if (t->nodes[at].key == key)
            return at;
        at = key < t->nodes[at].key ? t->nodes[at].left : t->nodes[at].right;
    }
    return -1;
}

static int height(const struct cd_tree *t, int at)
{
    return at < 0 ? 0 : t->nodes[at].height;
}

static void update(struct cd_tree *t, int at)
{
    int left = height(t, t->nodes[at].left),
        right = height(t, t->nodes[at].right);
    t->nodes[at].height = 1 + (left > right ? left : right);
}

static void replace(struct cd_tree *t, int old, int next)
{
    int parent = t->nodes[old].parent;
    if (parent < 0)
        t->root = next;
    else if (t->nodes[parent].left == old)
        t->nodes[parent].left = next;
    else
        t->nodes[parent].right = next;
    if (next >= 0)
        t->nodes[next].parent = parent;
}

static int left_rotate(struct cd_tree *t, int at)
{
    ++t->rotations;
    int next = t->nodes[at].right, inner = t->nodes[next].left;
    replace(t, at, next);
    t->nodes[next].left = at;
    t->nodes[at].parent = next;
    t->nodes[at].right = inner;
    if (inner >= 0)
        t->nodes[inner].parent = at;
    if (CD_MUTANT != 5)
        update(t, at);
    update(t, next);
    return next;
}

static int right_rotate(struct cd_tree *t, int at)
{
    ++t->rotations;
    int next = t->nodes[at].left, inner = t->nodes[next].right;
    replace(t, at, next);
    t->nodes[next].right = at;
    t->nodes[at].parent = next;
    t->nodes[at].left = inner;
    if (inner >= 0)
        t->nodes[inner].parent = at;
    update(t, at);
    update(t, next);
    return next;
}

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
            int left = t->nodes[at].left;
            if (height(t, t->nodes[left].left) <
                height(t, t->nodes[left].right))
                left_rotate(t, left);
            at = right_rotate(t, at);
        }
        else if (t->balanced && difference < -1)
        {
            int right = t->nodes[at].right;
            if (height(t, t->nodes[right].right) <
                height(t, t->nodes[right].left))
                right_rotate(t, right);
            at = left_rotate(t, at);
        }
        at = t->nodes[at].parent;
    }
}

enum cd_status cd_tree_insert(struct cd_tree *t, int key, unsigned fail,
                              int *out)
{
    if (!t || !out)
        return CD_INVALID;
    int found = cd_tree_find(t, key);
    if (found >= 0)
    {
        if (!t->multiplicity)
            return CD_DUPLICATE;
        if (t->nodes[found].multiplicity == UINT32_MAX)
            return CD_FULL;
        ++t->nodes[found].multiplicity;
        *out = found;
        return CD_OK;
    }
    if (fail)
        return CD_NOMEM;
    unsigned slot = 0;
    while (slot < 32 &&
           (t->nodes[slot].live || t->nodes[slot].generation == UINT32_MAX))
        ++slot;
    if (slot == 32)
        return CD_FULL;
    int parent = -1, at = t->root;
    while (at >= 0)
    {
        parent = at;
        at = key < t->nodes[at].key ? t->nodes[at].left : t->nodes[at].right;
    }
    uint32_t generation = t->nodes[slot].generation;
    t->nodes[slot] =
        (struct cd_tree_node){key, -1, -1, parent, 1, 1, 1, generation};
    if (parent < 0)
        t->root = (int)slot;
    else if (key < t->nodes[parent].key)
        t->nodes[parent].left = (int)slot;
    else
        t->nodes[parent].right = (int)slot;
    ++t->count;
    repair(t, parent);
    *out = (int)slot;
    return CD_OK;
}

enum cd_status cd_tree_delete(struct cd_tree *t, int key)
{
    if (!t)
        return CD_INVALID;
    int at = cd_tree_find(t, key);
    if (at < 0)
        return CD_MISSING;
    struct cd_tree_node *n = &t->nodes[at];
    if (n->multiplicity > 1)
    {
        --n->multiplicity;
        return CD_OK;
    }
    int start = n->parent;
    if (n->left < 0)
        replace(t, at, n->right);
    else if (n->right < 0)
        replace(t, at, n->left);
    else
    {
        int successor = n->right;
        while (t->nodes[successor].left >= 0)
            successor = t->nodes[successor].left;
        if (t->nodes[successor].parent != at)
        {
            start = t->nodes[successor].parent;
            replace(t, successor, t->nodes[successor].right);
            t->nodes[successor].right = n->right;
            t->nodes[n->right].parent = successor;
        }
        else
            start = successor;
        replace(t, at, successor);
        t->nodes[successor].left = n->left;
        t->nodes[n->left].parent = successor;
        update(t, successor);
    }
    n->live = 0;
    n->left = n->right = n->parent = -1;
    n->height = 0;
    n->multiplicity = 0;
    if (n->generation < UINT32_MAX)
        ++n->generation;
    --t->count;
    repair(t, start);
    return CD_OK;
}

enum cd_status cd_tree_walk(const struct cd_tree *t, unsigned *stack,
                            size_t stack_cap, int *out, size_t capacity,
                            size_t *used)
{
    if (!t || !used || (!stack && stack_cap) || (!out && capacity) ||
        stack_cap > 32)
        return CD_INVALID;
    size_t top = 0, written = 0;
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
        for (unsigned i = 0; i < t->nodes[at].multiplicity; ++i)
        {
            if (written == capacity)
            {
                *used = written;
                return CD_FULL;
            }
            out[written++] = t->nodes[at].key;
        }
        at = t->nodes[at].right;
    }
    *used = written;
    return CD_OK;
}

enum cd_status cd_tree_parent_walk(const struct cd_tree *t, int *out,
                                   size_t capacity, size_t *used)
{
    if (!t || !used || (!out && capacity))
        return CD_INVALID;
    size_t written = 0;
    int at = t->root;
    if (at >= 0)
        while (t->nodes[at].left >= 0)
            at = t->nodes[at].left;
    while (at >= 0)
    {
        for (unsigned i = 0; i < t->nodes[at].multiplicity; ++i)
        {
            if (written == capacity)
            {
                *used = written;
                return CD_FULL;
            }
            out[written++] = t->nodes[at].key;
        }
        if (t->nodes[at].right >= 0)
        {
            at = t->nodes[at].right;
            while (t->nodes[at].left >= 0)
                at = t->nodes[at].left;
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
