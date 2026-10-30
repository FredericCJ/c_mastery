#ifndef CMASTERY_ALGO_H
#define CMASTERY_ALGO_H
#include <stddef.h>
#include <stdint.h>

enum
{
    ALGO_OK = 0,
    ALGO_BAD = 1,
    ALGO_FULL = 2,
    ALGO_MISSING = 3,
    ALGO_OVERFLOW = 4,
    ALGO_UNIMPLEMENTED = 5
};

typedef struct
{
    uint64_t comparisons, moves, visits;
} algo_work;

typedef struct
{
    uint32_t key, tag;
} algo_pair;

_Static_assert(sizeof(algo_pair) == 8,
               "assembly record stride requires 8 bytes");

typedef struct
{
    int32_t data[16];
    uint32_t head, count, capacity;
} algo_ring;

int algo_ring_init(algo_ring *, uint32_t);
int algo_ring_put(algo_ring *, int32_t);
int algo_ring_take(algo_ring *, int32_t *);

typedef struct
{
    uint32_t key;
    int32_t value;
    uint8_t state;
} algo_bucket;

typedef struct
{
    algo_bucket slots[16];
    uint32_t count;
} algo_hash;

void algo_hash_init(algo_hash *);
int algo_hash_put(algo_hash *, uint32_t, int32_t, algo_work *);
int algo_hash_get(const algo_hash *, uint32_t, int32_t *, algo_work *);
int algo_hash_delete(algo_hash *, uint32_t, algo_work *);

typedef struct
{
    int32_t key;
    int16_t left, right, height;
    uint8_t live;
} algo_node;

typedef struct
{
    algo_node nodes[64];
    int16_t root;
    uint16_t count;
} algo_avl;

void algo_avl_init(algo_avl *);
int algo_avl_insert(algo_avl *, int32_t);
int algo_avl_delete(algo_avl *, int32_t);
int algo_avl_contains(const algo_avl *, int32_t);

typedef struct
{
    uint8_t parent[64], rank[64], count;
} algo_sets;

int algo_sets_init(algo_sets *, uint32_t);
int algo_sets_union(algo_sets *, uint32_t, uint32_t);
int algo_sets_find(algo_sets *, uint32_t, uint32_t *);
#define ALGO_INF UINT32_MAX

typedef struct
{
    uint32_t n, weight[16][16];
} algo_graph;

int algo_bfs(const algo_graph *, uint32_t, uint32_t distances[16], algo_work *);
int algo_dijkstra(const algo_graph *, uint32_t, uint32_t distances[16],
                  algo_work *);

int algo_lower(const uint32_t *, uint32_t, uint32_t, uint32_t *, algo_work *);
int algo_insertion(algo_pair *, uint32_t, uint32_t capacity, algo_work *);
int algo_merge(algo_pair *, uint32_t, uint32_t capacity, algo_work *);
int algo_sift(uint32_t *, uint32_t, uint32_t capacity, uint32_t p, algo_work *);
int algo_match(const uint8_t *, size_t, const uint8_t *, size_t, size_t *,
               algo_work *);

typedef struct
{
    uint8_t *data;
    size_t capacity, used;
} algo_arena;

int algo_arena_init(algo_arena *, uint8_t *, size_t);
int algo_arena_take(algo_arena *, size_t bytes, size_t alignment,
                    size_t *offset);
int algo_arena_rewind(algo_arena *, size_t mark);

typedef struct
{
    int32_t value[16];
    uint32_t generation[16];
    uint8_t state[16];
} algo_pool;

void algo_pool_init(algo_pool *);
int algo_pool_take(algo_pool *, int32_t, uint64_t *handle);
int algo_pool_get(const algo_pool *, uint64_t, int32_t *);
int algo_pool_release(algo_pool *, uint64_t);
int algo_sum(const double *, uint32_t, double *);
int algo_q7(const int8_t *, const int8_t *, uint32_t, int8_t *);

typedef struct
{
    uint8_t value, carry, overflow;
} algo_alu_result;

algo_alu_result algo_alu4(uint8_t a, uint8_t b, uint8_t carry);

typedef struct
{
    uint32_t misses, hits, compulsory, conflict, capacity;
} algo_cache_result;

int algo_cache(const uint32_t *, uint32_t, algo_cache_result *);

/* Trusted assembly cores: caller checks extent, ordering, capacity, heap
 * premise. SysV x86-64 Linux or base scalar AAPCS32 Thumb only. */
uint32_t algo_asm_lower(const uint32_t *, uint32_t, uint32_t);
void algo_asm_insert(algo_pair *, uint32_t);
void algo_asm_sift(uint32_t *, uint32_t, uint32_t);
/* Probe all callee-preserved integer registers across a nested lower-bound
 * call. Returns 1 on ABI success and stores lower-bound result in *out. */
int algo_abi_probe(const uint32_t *, uint32_t, uint32_t, uint32_t *out);
int algo_checked_lower(const uint32_t *, uint32_t, uint32_t, uint32_t,
                       uint32_t *);
int algo_checked_insert(algo_pair *, uint32_t, uint32_t);
int algo_checked_sift(uint32_t *, uint32_t, uint32_t, uint32_t);
#endif
