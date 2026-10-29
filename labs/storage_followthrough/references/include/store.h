#ifndef CMASTERY_STORE_H
#define CMASTERY_STORE_H
#include <stddef.h>
#include <stdint.h>
#define STORE_PAGE_SIZE 4096u
#define STORE_DATA_PAGES 32u
#define STORE_IMAGE_PAGES (3u + 2u * STORE_DATA_PAGES)
#define STORE_KEY_MAX 16u
#define STORE_VALUE_MAX 64u
#ifndef TREE_CAPACITY
#define TREE_CAPACITY 7u
#endif
_Static_assert(TREE_CAPACITY==7u,"CST5 capacity7 only");
#define STORE_NODE_MAX TREE_CAPACITY
#define STORE_CACHE_PAGES 4u
enum store_result {
  STORE_OK = 0,
  STORE_NOT_FOUND = 1,
  STORE_INVALID = 2,
  STORE_FULL = 3,
  STORE_IO = 4,
  STORE_CORRUPT = 5,
  STORE_STATE = 6,
  STORE_INDETERMINATE = 7,
  STORE_UNIMPLEMENTED = 8, STORE_STALE = 9
};
typedef struct store_io {
  void *ctx;
  int (*read)(void *ctx, uint32_t page, void *bytes);
  int (*write)(void *ctx, uint32_t page, const void *bytes);
  int (*flush)(void *ctx);
  uint32_t page_count;
} store_io;
typedef struct store_node {
  uint8_t leaf, n, klen[8], vlen[8];
  uint32_t next, child[9];
  uint64_t incarnation,next_incarnation,child_incarnation[9];
  uint8_t key[8][STORE_KEY_MAX], value[8][STORE_VALUE_MAX];
} store_node;
typedef struct store_cache_entry {
  uint32_t page;
  int valid;
  uint8_t bytes[STORE_PAGE_SIZE];
} store_cache_entry;
typedef struct store_store {
  store_io io;
  uint64_t generation, checkpoint_generation;
  uint32_t root, used, saved_root, saved_used;
  int active, poisoned, wal_active;
  store_node scratch[STORE_DATA_PAGES];
  store_cache_entry cache[STORE_CACHE_PAGES];
  uint64_t reads, writes, flushes, cache_hits;
  uint32_t touched;
  uint8_t touched_ids[31], touched_count;
  uint64_t root_incarnation,saved_root_incarnation,incarnations[32],saved_incarnations[32];
} store;
/* Caller serializes all access. No concurrent readers/writers or aliasing
 * callbacks. read/write transfer one page completely or report error. On error,
 * partial persistence is possible. Successful flush makes every prior write
 * durable. */
int store_format(store_io io);
int store_open(store *s, store_io io);
int store_begin(store *s);
void store_abort(store *s);
int store_put(store *s, const void *key, size_t klen, const void *value,
              size_t vlen);
int store_delete(store *s, const void *key, size_t klen);
int store_get(store *s, const void *key, size_t klen, void *value,
              size_t capacity, size_t *length);
typedef int (*store_visit)(void *ctx, const uint8_t *key, size_t klen,
                           const uint8_t *value, size_t vlen);
int store_scan(store *s, const void *lower, size_t lower_length,
               store_visit visit, void *ctx);
int store_commit(store *s);
int store_checkpoint(store *s);
int store_check(store *s);
/* Progressive component interfaces. Node pages use logical IDs 1..31.
 * Codec rejection preserves output. Cache study may use a zero-initialized
 * store with only io installed and wal_active=0; higher-level operations need
 * store_open or the explicit valid component-state fixtures from the tests. */
int store_page_encode(uint32_t id, uint64_t generation, const store_node *node,
                      uint8_t bytes[STORE_PAGE_SIZE]);
int store_page_decode(uint32_t id, uint64_t generation,
                      const uint8_t bytes[STORE_PAGE_SIZE], store_node *node);
int store_page_read(store *s, uint32_t id, uint8_t bytes[STORE_PAGE_SIZE]);
#endif
