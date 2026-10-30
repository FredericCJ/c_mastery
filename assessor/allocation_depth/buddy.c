#include "allocation.h"

/*
 * The 64-byte region is a complete binary tree stored in array order: node
 * 0 is the whole region (64 bytes), node i has children 2i+1 and 2i+2 of half
 * the size, and the eight leaves are 8 bytes. A node is FREE, SPLIT (its
 * children are managed separately; it has no id) or ALLOCATED (it has an id
 * and its subtree is untouched).
 */
enum buddy_geometry
{
    REGION_BYTES = 64,
    LEAF_BYTES = 8
};

enum node_state
{
    NODE_FREE = 0,
    NODE_SPLIT = 1,
    NODE_ALLOCATED = 2
};

static unsigned left_child(unsigned node)
{
    return node * 2 + 1;
}

static unsigned right_child(unsigned node)
{
    return node * 2 + 2;
}

int al_buddy_init(struct al_buddy *b)
{
    if (!b)
    {
        return AL_BAD;
    }
    *b = (struct al_buddy){0};
    return AL_OK;
}

/* Structural validation of the subtree rooted at `node` of `size` bytes. */
static int valid(const struct al_buddy *b, unsigned node, unsigned size)
{
    if (b->state[node] > NODE_ALLOCATED)
    {
        return 0;
    }
    unsigned left = left_child(node);
    unsigned right = right_child(node);
    if (b->state[node] == NODE_SPLIT)
    {
        if (size == LEAF_BYTES || b->ids[node])
        {
            return 0;
        }
        return valid(b, left, size / 2) && valid(b, right, size / 2);
    }
    if ((b->state[node] == NODE_ALLOCATED) != (b->ids[node] != 0))
    {
        return 0;
    }
    if (size > LEAF_BYTES)
    {
        if (b->state[left] || b->state[right] || b->ids[left] || b->ids[right])
        {
            return 0;
        }
        return valid(b, left, size / 2) && valid(b, right, size / 2);
    }
    return 1;
}

int al_buddy_check(const struct al_buddy *b)
{
    return b && valid(b, 0, REGION_BYTES) ? AL_OK : AL_CORRUPT;
}

/* Where a successful search placed the request. */
struct buddy_choice
{
    unsigned node;
    unsigned offset; /* bytes from the start of the region */
    unsigned size; /* bytes charged: the node's size */
};

/*
 * Depth-first search for a free node of exactly `need` bytes, splitting nodes
 * on the way down; a branch that fails is merged back to FREE while its
 * children are free. The caller runs it on a scratch copy and keeps the copy
 * only when it returns 1, which also fills *choice.
 */
static int take(struct al_buddy *b, unsigned node, unsigned size, unsigned off,
                unsigned need, struct buddy_choice *choice)
{
    ++b->steps;
    if (b->state[node] == NODE_ALLOCATED)
    {
        return 0;
    }
    if (size == need)
    {
        if (b->state[node])
        {
            return 0;
        }
        b->state[node] = NODE_ALLOCATED;
        b->ids[node] = ++b->serial;
        *choice = (struct buddy_choice){node, off, size};
        return 1;
    }
    b->state[node] = NODE_SPLIT;
    unsigned left = left_child(node);
    unsigned right = right_child(node);
    if (take(b, left, size / 2, off, need, choice) ||
        take(b, right, size / 2, off + size / 2, need, choice))
    {
        return 1;
    }
    if (!b->state[left] && !b->state[right])
    {
        b->state[node] = NODE_FREE;
    }
    return 0;
}

/* Smallest power of two >= n that is also >= one leaf. */
static unsigned rounded_size(size_t n)
{
    unsigned need = LEAF_BYTES;
    while (need < n)
    {
        need *= 2;
    }
    return need;
}

int al_buddy_take(struct al_buddy *b, size_t n, struct al_handle *out,
                  size_t *offset, size_t *charged)
{
    if (!b || !out || !offset || !charged || !n || n > REGION_BYTES)
    {
        return AL_BAD;
    }
    if (al_buddy_check(b))
    {
        return AL_CORRUPT;
    }
    if (b->serial == UINT32_MAX)
    {
        return AL_RETIRED;
    }
    struct al_buddy scratch = *b;
    struct buddy_choice choice;
    scratch.steps = 0;
    if (!take(&scratch, 0, REGION_BYTES, 0, rounded_size(n), &choice))
    {
        return AL_FULL;
    }
    *b = scratch;
    *out = (struct al_handle){b, b->ids[choice.node]};
    *offset = choice.offset;
    *charged = choice.size;
    return AL_OK;
}

int al_buddy_release(struct al_buddy *b, struct al_handle h)
{
    if (!b)
    {
        return AL_BAD;
    }
    if (al_buddy_check(b))
    {
        return AL_CORRUPT;
    }
    if (h.owner != b || !h.id)
    {
        return AL_STALE;
    }
    unsigned node = AL_BUDDY_NODES;
    for (unsigned i = 0; i < AL_BUDDY_NODES; ++i)
    {
        if (b->state[i] == NODE_ALLOCATED && b->ids[i] == h.id)
        {
            node = i;
            break;
        }
    }
    if (node == AL_BUDDY_NODES)
    {
        return AL_STALE;
    }
    b->state[node] = NODE_FREE;
    b->ids[node] = 0;
    b->steps = 1;
    /* Climb while the buddy subtree is also entirely free, merging upward. */
    while (node)
    {
        unsigned parent = (node - 1) / 2;
        unsigned left = left_child(parent);
        unsigned right = right_child(parent);
        ++b->steps;
#if AL_MUTANT == 7
        if (b->state[left] && b->state[right])
        {
            break;
        }
#else
        if (b->state[left] || b->state[right])
        {
            break;
        }
#endif
        b->state[parent] = NODE_FREE;
        node = parent;
    }
    return AL_OK;
}
