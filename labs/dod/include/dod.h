#ifndef CMASTERY_DOD_H
#define CMASTERY_DOD_H
#include <stddef.h>
#include <stdint.h>

enum d_status
{
    D_OK,
    D_INVALID,
    D_FULL,
    D_RANGE,
    D_STALE,
    D_FAILED
};

struct d_allocator
{
    void *context;
    unsigned char *(*allocate)(void *, size_t);
    void (*release)(void *, unsigned char *);
};

struct d_small
{
    unsigned char local[16];
    unsigned char *heap;
    size_t length, capacity;
    struct d_allocator allocator;
};

void d_small_init(struct d_small *, struct d_allocator);
int d_small_append(struct d_small *, const unsigned char *, size_t);
int d_small_truncate(struct d_small *, size_t);
int d_small_compact(struct d_small *);
int d_small_move(struct d_small *destination, struct d_small *source);
void d_small_reset(struct d_small *);
const unsigned char *d_small_data(const struct d_small *);

struct d_columns
{
    int32_t x[4], velocity[4];
    unsigned cold[4];
    size_t count;
};

int d_columns_step(struct d_columns *, int32_t ticks);
uint32_t d_swar_add(uint32_t, uint32_t);

struct d_slot
{
    unsigned generation, live;
    int value;
};

struct d_sparse
{
    struct d_slot slots[4];
    unsigned dense[4], rank[4], count;
};

struct d_handle
{
    const struct d_sparse *owner;
    unsigned slot, generation;
};

void d_sparse_init(struct d_sparse *);
int d_sparse_add(struct d_sparse *, int, struct d_handle *);
int d_sparse_get(const struct d_sparse *, struct d_handle, int *);
int d_sparse_remove(struct d_sparse *, struct d_handle);

struct d_table
{
    unsigned ids[2], count;
    int position[2], velocity[2];
};

struct d_ecs
{
    struct d_table tables[3];
    unsigned mask[4], row[4];
};

void d_ecs_init(struct d_ecs *);
int d_ecs_set(struct d_ecs *, unsigned id, unsigned mask, int position,
              int velocity);
int d_ecs_step(struct d_ecs *);

struct d_tree_node
{
    unsigned left, right;
    int value;
};

struct d_tree
{
    struct d_tree_node nodes[65];
    unsigned used, fail_after, allocations;
};

void d_tree_init(struct d_tree *);
int d_tree_update(struct d_tree *, unsigned root, unsigned index, int value,
                  unsigned *new_root);
int d_tree_get(const struct d_tree *, unsigned root, unsigned index, int *);

struct d_hamt_node
{
    unsigned bitmap, children[4], count, keys[2];
    int values[2];
};

struct d_hamt
{
    struct d_hamt_node nodes[65];
    unsigned used, fail_after, allocations;
};

void d_hamt_init(struct d_hamt *);
int d_hamt_update(struct d_hamt *, unsigned root, unsigned key, int value,
                  unsigned *new_root);
int d_hamt_get(const struct d_hamt *, unsigned root, unsigned key, int *);

struct d_rope_node
{
    unsigned left, right, length;
    unsigned char bytes[8];
};

struct d_rope
{
    struct d_rope_node nodes[65];
    unsigned used;
};

void d_rope_init(struct d_rope *);
int d_rope_leaf(struct d_rope *, const unsigned char *, size_t, unsigned *);
int d_rope_concat(struct d_rope *, unsigned, unsigned, unsigned *);
int d_rope_flatten(const struct d_rope *, unsigned, unsigned char *, size_t,
                   size_t *);
#endif
