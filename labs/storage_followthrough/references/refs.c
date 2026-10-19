/* Learner implementation. Build succeeds; acceptance tests intentionally fail.
 * Implement the mechanism here using include/store.h and HINTS.md. */
#include "refs.h"
int store_page_encode(uint32_t id, uint64_t generation, const store_node *node,
                      uint8_t bytes[STORE_PAGE_SIZE]) {
  (void)id;
  (void)generation;
  (void)node;
  (void)bytes;
  return STORE_UNIMPLEMENTED;
}
int store_page_decode(uint32_t id, uint64_t generation,
                      const uint8_t bytes[STORE_PAGE_SIZE], store_node *node) {
  (void)id;
  (void)generation;
  (void)bytes;
  (void)node;
  return STORE_UNIMPLEMENTED;
}
int store_page_read(store *s, uint32_t id, uint8_t bytes[STORE_PAGE_SIZE]) {
  (void)s;
  (void)id;
  (void)bytes;
  return STORE_UNIMPLEMENTED;
}
int store_format(store_io io) {
  (void)io;
  return STORE_UNIMPLEMENTED;
}
int store_open(store *s, store_io io) {
  (void)s;
  (void)io;
  return STORE_UNIMPLEMENTED;
}
int store_begin(store *s) {
  (void)s;
  return STORE_UNIMPLEMENTED;
}
void store_abort(store *s) { (void)s; }
int store_put(store *s, const void *k, size_t kl, const void *v, size_t vl) {
  (void)s;
  (void)k;
  (void)kl;
  (void)v;
  (void)vl;
  return STORE_UNIMPLEMENTED;
}
int store_delete(store *s, const void *k, size_t kl) {
  (void)s;
  (void)k;
  (void)kl;
  return STORE_UNIMPLEMENTED;
}
int store_get(store *s, const void *k, size_t kl, void *v, size_t cap,
              size_t *n) {
  (void)s;
  (void)k;
  (void)kl;
  (void)v;
  (void)cap;
  (void)n;
  return STORE_UNIMPLEMENTED;
}
int store_scan(store *s, const void *k, size_t kl, store_visit fn, void *ctx) {
  (void)s;
  (void)k;
  (void)kl;
  (void)fn;
  (void)ctx;
  return STORE_UNIMPLEMENTED;
}
int store_commit(store *s) {
  (void)s;
  return STORE_UNIMPLEMENTED;
}
int store_checkpoint(store *s) {
  (void)s;
  return STORE_UNIMPLEMENTED;
}
int store_check(store *s) {
  (void)s;
  return STORE_UNIMPLEMENTED;
}

int store_ref_make(store *s,uint32_t id,StoreRef *out){(void)s;(void)id;(void)out;return STORE_UNIMPLEMENTED;}
int store_ref_read(StoreRef ref,store_node *out){(void)ref;(void)out;return STORE_UNIMPLEMENTED;}
