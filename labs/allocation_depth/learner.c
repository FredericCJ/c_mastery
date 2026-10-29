#include "allocation.h"
int al_extent(size_t c, size_t n, size_t a, size_t cap, size_t *at,
              size_t *end) {
  (void)c;
  (void)n;
  (void)a;
  (void)cap;
  (void)at;
  (void)end;
  return AL_BAD;
}
int al_arena_init(struct al_arena *a, unsigned char *p, size_t cap, size_t align) {
  (void)a;
  (void)p;
  (void)cap;
  (void)align;
  return AL_BAD;
}
int al_arena_take(struct al_arena *a, size_t n, size_t align, al_cleanup_fn f, void *ctx,
                  struct al_handle *out) {
  (void)a;
  (void)n;
  (void)align;
  (void)f;
  (void)ctx;
  (void)out;
  return AL_BAD;
}
int al_arena_view(struct al_arena *a, struct al_handle h, unsigned char **out, size_t *size) {
  (void)a;
  (void)h;
  (void)out;
  (void)size;
  return AL_BAD;
}
int al_arena_mark(struct al_arena *a, struct al_mark *out) {
  (void)a;
  (void)out;
  return AL_BAD;
}
int al_arena_rollback(struct al_arena *a, struct al_mark mark) {
  (void)a;
  (void)mark;
  return AL_BAD;
}
int al_arena_commit(struct al_arena *a, struct al_mark mark) {
  (void)a;
  (void)mark;
  return AL_BAD;
}
int al_arena_release(struct al_arena *a, struct al_handle h) {
  (void)a;
  (void)h;
  return AL_BAD;
}
int al_arena_reset(struct al_arena *a) {
  (void)a;
  return AL_BAD;
}
int al_buddy_init(struct al_buddy *b) {
  (void)b;
  return AL_BAD;
}
int al_buddy_check(const struct al_buddy *b) {
  (void)b;
  return AL_BAD;
}
int al_buddy_take(struct al_buddy *b, size_t n, struct al_handle *out, size_t *offset,
                  size_t *charged) {
  (void)b;
  (void)n;
  (void)out;
  (void)offset;
  (void)charged;
  return AL_BAD;
}
int al_buddy_release(struct al_buddy *b, struct al_handle h) {
  (void)b;
  (void)h;
  return AL_BAD;
}
int al_stable_init(struct al_stable *s, struct al_heap *heap) {
  (void)s;
  (void)heap;
  return AL_BAD;
}
int al_stable_take(struct al_stable *s, size_t n, struct al_stable_handle *out) {
  (void)s;
  (void)n;
  (void)out;
  return AL_BAD;
}
int al_stable_view(struct al_stable *s, struct al_stable_handle h, unsigned char **out,
                   size_t *n) {
  (void)s;
  (void)h;
  (void)out;
  (void)n;
  return AL_BAD;
}
int al_stable_resize(struct al_stable *s, struct al_stable_handle h, size_t n, int fail) {
  (void)s;
  (void)h;
  (void)n;
  (void)fail;
  return AL_BAD;
}
int al_stable_release(struct al_stable *s, struct al_stable_handle h) {
  (void)s;
  (void)h;
  return AL_BAD;
}
int al_extent_choose(struct al_extent *a, size_t *count, size_t bytes, size_t minimum,
                     unsigned best, size_t *offset, size_t *charged) {
  (void)a;
  (void)count;
  (void)bytes;
  (void)minimum;
  (void)best;
  (void)offset;
  (void)charged;
  return AL_BAD;
}
int al_extent_coalesce(struct al_extent *a, size_t *count, size_t capacity) {
  (void)a;
  (void)count;
  (void)capacity;
  return AL_BAD;
}
int al_heap_api(struct al_heap *h, struct al_allocator *out) {
  (void)h;
  (void)out;
  return AL_BAD;
}
int al_api_allocate(const struct al_allocator *a, size_t n, size_t align,
                    struct al_handle *out) {
  (void)a;
  (void)n;
  (void)align;
  (void)out;
  return AL_BAD;
}
int al_heap_check(const struct al_heap *h) {
  (void)h;
  return AL_BAD;
}
int al_heap_init(struct al_heap *h, unsigned n, unsigned best, unsigned deferred,
                 unsigned instrument) {
  (void)h;
  (void)n;
  (void)best;
  (void)deferred;
  (void)instrument;
  return AL_BAD;
}
int al_heap_take(struct al_heap *h, size_t n, size_t alignment, struct al_handle *out) {
  (void)h;
  (void)n;
  (void)alignment;
  (void)out;
  return AL_BAD;
}
int al_heap_view(struct al_heap *h, struct al_handle handle, unsigned char **out, size_t *n) {
  (void)h;
  (void)handle;
  (void)out;
  (void)n;
  return AL_BAD;
}
int al_heap_release(struct al_heap *h, struct al_handle handle) {
  (void)h;
  (void)handle;
  return AL_BAD;
}
int al_heap_collect(struct al_heap *h) {
  (void)h;
  return AL_BAD;
}
int al_heap_stats(const struct al_heap *h, size_t *total, size_t *largest) {
  (void)h;
  (void)total;
  (void)largest;
  return AL_BAD;
}
int al_heap_resize_copy(struct al_heap *h, struct al_handle handle, size_t n, int fail_acquire,
                        al_copy_fn copier, void *context, struct al_handle *out) {
  (void)h;
  (void)handle;
  (void)n;
  (void)fail_acquire;
  (void)copier;
  (void)context;
  (void)out;
  return AL_BAD;
}
int al_heap_resize(struct al_heap *h, struct al_handle handle, size_t n, int fail_acquire,
                   size_t fail_after, struct al_handle *out) {
  (void)h;
  (void)handle;
  (void)n;
  (void)fail_acquire;
  (void)fail_after;
  (void)out;
  return AL_BAD;
}
void al_pool_init(struct al_pool *p) {
  (void)p;
  return;
}
int al_pool_take(struct al_pool *p, int32_t value, struct al_pool_handle *out) {
  (void)p;
  (void)value;
  (void)out;
  return AL_BAD;
}
int al_pool_get(const struct al_pool *p, struct al_pool_handle h, int32_t *out) {
  (void)p;
  (void)h;
  (void)out;
  return AL_BAD;
}
int al_pool_release(struct al_pool *p, struct al_pool_handle h) {
  (void)p;
  (void)h;
  return AL_BAD;
}
int al_classes_init(struct al_classes *c) {
  (void)c;
  return AL_BAD;
}
int al_classes_take(struct al_classes *c, size_t n,
                    int (*init)(void *, unsigned char *, size_t),
                    al_cleanup_fn destroy, void *ctx, struct al_handle *out) {
  (void)c;
  (void)n;
  (void)init;
  (void)destroy;
  (void)ctx;
  (void)out;
  return AL_BAD;
}
int al_classes_release(struct al_classes *c, struct al_handle h) {
  (void)c;
  (void)h;
  return AL_BAD;
}
int al_classes_stats(const struct al_classes *c, size_t *payload, size_t *charged,
                     size_t *retained) {
  (void)c;
  (void)payload;
  (void)charged;
  (void)retained;
  return AL_BAD;
}
