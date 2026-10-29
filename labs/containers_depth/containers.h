#ifndef CMASTERY_CONTAINERS_DEPTH_H
#define CMASTERY_CONTAINERS_DEPTH_H
#include <stddef.h>
#include <stdint.h>

enum cd_status
{
    CD_OK,
    CD_INVALID,
    CD_FULL,
    CD_EMPTY,
    CD_NOMEM,
    CD_BUSY,
    CD_FAILED,
    CD_MISSING,
    CD_DUPLICATE
};

struct cd_array
{
    int *data;
    size_t length, capacity;
    unsigned live;
};
enum cd_status cd_array_init(struct cd_array *, int *, size_t);
enum cd_status cd_array_insert(struct cd_array *, size_t, const int *);
enum cd_status cd_array_erase(struct cd_array *, size_t, int *);
enum cd_status cd_array_get(const struct cd_array *, size_t, int *);
enum cd_status cd_array_destroy(struct cd_array *);

struct cd_view
{
    const int *base;
    size_t extent, first, count;
    ptrdiff_t stride;
};
enum cd_status cd_view_init(struct cd_view *, const int *, size_t, size_t,
                            size_t, ptrdiff_t);
enum cd_status cd_view_get(const struct cd_view *, size_t, int *);
enum cd_status cd_view_sub(const struct cd_view *, size_t, size_t,
                           struct cd_view *);

struct cd_ring
{
    int *data;
    size_t capacity, head, count, reserved;
    unsigned reserving;
};

struct cd_spans
{
    int *first;
    size_t first_count;
    int *second;
    size_t second_count;
};
enum cd_status cd_ring_init(struct cd_ring *, int *, size_t);
enum cd_status cd_ring_read(const struct cd_ring *, struct cd_spans *);
enum cd_status cd_ring_reserve(struct cd_ring *, size_t, struct cd_spans *);
enum cd_status cd_ring_commit(struct cd_ring *, size_t);
enum cd_status cd_ring_push(struct cd_ring *, const int *, size_t,
                            unsigned partial, size_t *);
enum cd_status cd_ring_pop(struct cd_ring *, int *, size_t);

struct cd_allocator
{
    void *context;
    int *(*take)(void *, size_t);
    void (*release)(void *, int *);
};

struct cd_vector
{
    int *data;
    size_t length, capacity;
    struct cd_allocator allocator;
    size_t copies;
    unsigned live;
};
enum cd_status cd_vector_init(struct cd_vector *, struct cd_allocator);
enum cd_status cd_vector_reserve(struct cd_vector *, size_t);
enum cd_status cd_vector_insert(struct cd_vector *, size_t, int);
enum cd_status cd_vector_erase(struct cd_vector *, size_t, int *);
enum cd_status cd_vector_shrink(struct cd_vector *);
enum cd_status cd_vector_destroy(struct cd_vector *);

struct cd_payload
{
    int value;
    unsigned resource;
};

struct cd_payload_ops
{
    void *context;
    int (*clone)(void *, const struct cd_payload *, struct cd_payload *);
    void (*destroy)(void *, struct cd_payload *);
};
enum cd_status cd_clone_batch(struct cd_payload *, const struct cd_payload *,
                              size_t, struct cd_payload_ops);

struct cd_payload_allocator
{
    void *context;
    struct cd_payload *(*take)(void *, size_t);
    void (*release)(void *, struct cd_payload *);
};

struct cd_payload_vector
{
    struct cd_payload *data;
    size_t count;
    struct cd_payload_allocator allocator;
    struct cd_payload_ops ops;
};
enum cd_status cd_payload_init(struct cd_payload_vector *,
                               struct cd_payload_allocator,
                               struct cd_payload_ops);
enum cd_status cd_payload_append(struct cd_payload_vector *,
                                 const struct cd_payload *, size_t);
enum cd_status cd_payload_destroy(struct cd_payload_vector *);
struct cd_link;

struct cd_list
{
    struct cd_link *head;
    struct cd_link *tail;
    size_t count;
};

struct cd_link
{
    struct cd_link *previous;
    struct cd_link *next;
    struct cd_list *owner;
    int value;
};

void cd_list_init(struct cd_list *);
enum cd_status cd_list_push(struct cd_list *, struct cd_link *, unsigned front);
enum cd_status cd_list_remove(struct cd_list *, struct cd_link *);

struct cd_circle
{
    struct cd_list identity;
    struct cd_link sentinel;
};

void cd_circle_init(struct cd_circle *);
enum cd_status cd_circle_append(struct cd_circle *, struct cd_link *);
enum cd_status cd_circle_remove(struct cd_circle *, struct cd_link *);

struct cd_single
{
    struct cd_single *next;
    int value;
    unsigned linked;
};

struct cd_single_list
{
    struct cd_single *head, *tail;
    size_t count;
};
enum cd_status cd_single_append(struct cd_single_list *, struct cd_single *);
enum cd_status cd_single_remove(struct cd_single_list *, struct cd_single *);

struct cd_heap_item
{
    int priority;
    unsigned identity, slot;
    uint32_t sequence;
};

struct cd_heap
{
    struct cd_heap_item data[16];
    unsigned index[16], generation[16], state[16], count, max_heap;
    uint32_t sequence;
    uint64_t comparisons;
};

struct cd_heap_handle
{
    const struct cd_heap *owner;
    unsigned slot, generation;
};

void cd_heap_init(struct cd_heap *, unsigned max_heap);
enum cd_status cd_heap_insert(struct cd_heap *, int, unsigned,
                              struct cd_heap_handle *);
enum cd_status cd_heap_take(struct cd_heap *, int *, unsigned *);
enum cd_status cd_heap_get(const struct cd_heap *, struct cd_heap_handle, int *,
                           unsigned *);
enum cd_status cd_heap_change(struct cd_heap *, struct cd_heap_handle, int);
enum cd_status cd_heap_build(struct cd_heap *, const int *, size_t,
                             unsigned max_heap);

struct cd_tree_node
{
    int key, left, right, parent, height;
    unsigned multiplicity, live;
    uint32_t generation;
};

struct cd_tree
{
    struct cd_tree_node nodes[32];
    int root;
    unsigned count, balanced, multiplicity, rotations, repair_steps;
};

void cd_tree_init(struct cd_tree *, unsigned balanced, unsigned multiplicity);
enum cd_status cd_tree_insert(struct cd_tree *, int, unsigned fail_acquisition,
                              int *slot);
enum cd_status cd_tree_delete(struct cd_tree *, int);
int cd_tree_find(const struct cd_tree *, int);
enum cd_status cd_tree_walk(const struct cd_tree *, unsigned *stack,
                            size_t stack_capacity, int *out, size_t capacity,
                            size_t *used);
enum cd_status cd_tree_parent_walk(const struct cd_tree *, int *, size_t,
                                   size_t *);

struct cd_bucket
{
    uint32_t key;
    int value;
    unsigned state;
};

struct cd_hash_allocator
{
    void *context;
    struct cd_bucket *(*take)(void *, unsigned);
    void (*release)(void *, struct cd_bucket *);
};

struct cd_hash
{
    struct cd_bucket *slots;
    unsigned capacity, live, occupied, clustered;
    struct cd_hash_allocator allocator;
};
enum cd_status cd_hash_init(struct cd_hash *, unsigned,
                            struct cd_hash_allocator, unsigned clustered);
enum cd_status cd_hash_put(struct cd_hash *, uint32_t, int, unsigned *probes);
enum cd_status cd_hash_get(const struct cd_hash *, uint32_t, int *,
                           unsigned *probes);
enum cd_status cd_hash_delete(struct cd_hash *, uint32_t);
enum cd_status cd_hash_rebuild(struct cd_hash *, unsigned);
enum cd_status cd_hash_destroy(struct cd_hash *);

struct cd_key
{
    unsigned char bytes[8];
    size_t length;
    int value;
};

struct cd_byte_map
{
    struct cd_key keys[4];
    unsigned live[4];
};
enum cd_status cd_bytemap_put(struct cd_byte_map *, const unsigned char *,
                              size_t, int);
enum cd_status cd_bytemap_get(const struct cd_byte_map *, const unsigned char *,
                              size_t, int *);

struct cd_trie_node
{
    int parent, child, sibling;
    unsigned char byte;
    unsigned live, terminal;
};

struct cd_trie
{
    struct cd_trie_node nodes[64];
    uint32_t generation;
    unsigned count;
};

struct cd_trie_cursor
{
    const struct cd_trie *owner;
    uint32_t generation;
    unsigned nodes[64], count, position;
};

void cd_trie_init(struct cd_trie *);
enum cd_status cd_trie_insert(struct cd_trie *, const unsigned char *, size_t,
                              unsigned fail_acquisition);
enum cd_status cd_trie_delete(struct cd_trie *, const unsigned char *, size_t);
enum cd_status cd_trie_prefix(const struct cd_trie *, const unsigned char *,
                              size_t, struct cd_trie_cursor *);
enum cd_status cd_trie_next(struct cd_trie_cursor *, struct cd_key *);

struct cd_sparse
{
    unsigned dense[16], sparse[32], tags[32], count, generation;
};

void cd_sparse_init(struct cd_sparse *);
int cd_sparse_has(const struct cd_sparse *, unsigned);
enum cd_status cd_sparse_insert(struct cd_sparse *, unsigned);
enum cd_status cd_sparse_remove(struct cd_sparse *, unsigned);
void cd_sparse_clear(struct cd_sparse *);

struct cd_undo
{
    unsigned field, index, old;
};

struct cd_sets
{
    unsigned parent[16], size[16], count;
    struct cd_undo log[128];
    size_t used;
};
enum cd_status cd_sets_init(struct cd_sets *, unsigned);
enum cd_status cd_sets_find(struct cd_sets *, unsigned, unsigned *);
enum cd_status cd_sets_union(struct cd_sets *, unsigned, unsigned);
enum cd_status cd_sets_rollback(struct cd_sets *, size_t);

struct cd_edge
{
    unsigned from, to;
    int weight;
};

struct cd_csr
{
    unsigned vertices, count, offset[9];
    struct cd_edge edges[32];
};
enum cd_status cd_csr_build(struct cd_csr *, unsigned, const struct cd_edge *,
                            size_t, unsigned parallel);
/* Separate path-algorithm consumer of the immutable representation. */
enum cd_status cd_csr_bfs(const struct cd_csr *, unsigned, uint32_t[8]);
enum cd_status cd_filter_insert(unsigned char *, unsigned);
enum cd_status cd_filter_query(unsigned char, unsigned, unsigned *);

struct cd_counting_filter
{
    unsigned char cells[8], keys[16];
};
enum cd_status cd_counting_insert(struct cd_counting_filter *, unsigned);
enum cd_status cd_counting_delete(struct cd_counting_filter *, unsigned);

struct cd_chain_node
{
    uint32_t key;
    int value, next;
    unsigned live;
};

struct cd_chain
{
    int head[4];
    struct cd_chain_node nodes[16];
    unsigned count;
};

void cd_chain_init(struct cd_chain *);
enum cd_status cd_chain_put(struct cd_chain *, uint32_t, int);
enum cd_status cd_chain_get(const struct cd_chain *, uint32_t, int *,
                            unsigned *);
enum cd_status cd_chain_delete(struct cd_chain *, uint32_t);
enum cd_status cd_bitset_mask(unsigned, uint32_t *);
enum cd_status cd_bitset_algebra(uint32_t, uint32_t, unsigned, uint32_t *,
                                 uint32_t *, uint32_t *);
enum cd_status cd_bitset_next(uint32_t, unsigned, unsigned *, unsigned *);

struct cd_queue_node
{
    int value, next;
    unsigned live;
};

struct cd_queue
{
    struct cd_queue_node nodes[6];
    int head, tail;
    unsigned count, available;
};

void cd_queue_init(struct cd_queue *);
enum cd_status cd_queue_push(struct cd_queue *, int, unsigned refill,
                             unsigned fail_refill);
enum cd_status cd_queue_pop(struct cd_queue *, int *);

struct cd_segmented
{
    int blocks[4][4];
    unsigned count;
};
enum cd_status cd_segmented_append(struct cd_segmented *, int, int **);

struct cd_move_only
{
    int value;
    unsigned resource;
};

struct cd_owned
{
    struct cd_move_only values[8];
    unsigned count;
};
enum cd_status cd_owned_append(struct cd_owned *, struct cd_move_only *,
                               unsigned cancellation_stage);

struct cd_deque
{
    int values[3];
    unsigned head, count;
};
enum cd_status cd_deque_push(struct cd_deque *, int, unsigned front);
enum cd_status cd_deque_pop(struct cd_deque *, int *, unsigned front);
#endif
