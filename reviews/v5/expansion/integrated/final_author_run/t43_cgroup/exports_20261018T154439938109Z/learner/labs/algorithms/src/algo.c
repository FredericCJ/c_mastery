/* Learner implementation. Every public mechanism is intentionally unfinished.
 * Replace one function at a time using SPEC.md and staged HINTS.md. */
#include "algo.h"
int algo_ring_init(algo_ring *q, uint32_t c) { return ALGO_UNIMPLEMENTED; }
int algo_ring_put(algo_ring *q, int32_t v) { return ALGO_UNIMPLEMENTED; }
int algo_ring_take(algo_ring *q, int32_t *v) { return ALGO_UNIMPLEMENTED; }
void algo_hash_init(algo_hash *h) {}
int algo_hash_put(algo_hash *h, uint32_t k, int32_t v, algo_work *w) {
  return ALGO_UNIMPLEMENTED;
}
int algo_hash_get(const algo_hash *h, uint32_t k, int32_t *v, algo_work *w) {
  return ALGO_UNIMPLEMENTED;
}
int algo_hash_delete(algo_hash *h, uint32_t k, algo_work *w) {
  return ALGO_UNIMPLEMENTED;
}
void algo_avl_init(algo_avl *t) {}
int algo_avl_insert(algo_avl *t, int32_t k) { return ALGO_UNIMPLEMENTED; }
int algo_avl_delete(algo_avl *t, int32_t k) { return ALGO_UNIMPLEMENTED; }
int algo_avl_contains(const algo_avl *t, int32_t k) { return 0; }
int algo_sets_init(algo_sets *s, uint32_t n) { return ALGO_UNIMPLEMENTED; }
int algo_sets_union(algo_sets *s, uint32_t a, uint32_t b) {
  return ALGO_UNIMPLEMENTED;
}
int algo_sets_find(algo_sets *s, uint32_t a, uint32_t *b) {
  return ALGO_UNIMPLEMENTED;
}
int algo_bfs(const algo_graph *g, uint32_t s, uint32_t d[16], algo_work *w) {
  return ALGO_UNIMPLEMENTED;
}
int algo_dijkstra(const algo_graph *g, uint32_t s, uint32_t d[16],
                  algo_work *w) {
  return ALGO_UNIMPLEMENTED;
}
int algo_lower(const uint32_t *a, uint32_t n, uint32_t k, uint32_t *p,
               algo_work *w) {
  return ALGO_UNIMPLEMENTED;
}
int algo_insertion(algo_pair *a, uint32_t n, uint32_t cap, algo_work *w) {
  return ALGO_UNIMPLEMENTED;
}
int algo_merge(algo_pair *a, uint32_t n, uint32_t cap, algo_work *w) {
  return ALGO_UNIMPLEMENTED;
}
int algo_sift(uint32_t *a, uint32_t n, uint32_t cap, uint32_t p, algo_work *w) {
  return ALGO_UNIMPLEMENTED;
}
int algo_match(const uint8_t *t, size_t n, const uint8_t *p, size_t m,
               size_t *out, algo_work *w) {
  return ALGO_UNIMPLEMENTED;
}
int algo_arena_init(algo_arena *a, uint8_t *d, size_t cap) {
  return ALGO_UNIMPLEMENTED;
}
int algo_arena_take(algo_arena *a, size_t n, size_t align, size_t *out) {
  return ALGO_UNIMPLEMENTED;
}
int algo_arena_rewind(algo_arena *a, size_t mark) { return ALGO_UNIMPLEMENTED; }
void algo_pool_init(algo_pool *p) {}
int algo_pool_take(algo_pool *p, int32_t v, uint64_t *h) {
  return ALGO_UNIMPLEMENTED;
}
int algo_pool_get(const algo_pool *p, uint64_t h, int32_t *v) {
  return ALGO_UNIMPLEMENTED;
}
int algo_pool_release(algo_pool *p, uint64_t h) { return ALGO_UNIMPLEMENTED; }
int algo_sum(const double *a, uint32_t n, double *out) {
  return ALGO_UNIMPLEMENTED;
}
int algo_q7(const int8_t *a, const int8_t *b, uint32_t n, int8_t *out) {
  return ALGO_UNIMPLEMENTED;
}
algo_alu_result algo_alu4(uint8_t a, uint8_t b, uint8_t c) {
  return (algo_alu_result){0, 0, 0};
}
int algo_cache(const uint32_t *a, uint32_t n, algo_cache_result *out) {
  return ALGO_UNIMPLEMENTED;
}
