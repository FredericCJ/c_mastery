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
int al_arena_init(ALArena *a, unsigned char *p, size_t cap, size_t align) {
  (void)a;
  (void)p;
  (void)cap;
  (void)align;
  return AL_BAD;
}
int al_arena_take(ALArena *a, size_t n, size_t align, ALCleanup f, void *ctx,
                  ALHandle *out) {
  (void)a;
  (void)n;
  (void)align;
  (void)f;
  (void)ctx;
  (void)out;
  return AL_BAD;
}
int al_arena_view(ALArena *a, ALHandle h, unsigned char **out, size_t *size) {
  (void)a;
  (void)h;
  (void)out;
  (void)size;
  return AL_BAD;
}
int al_arena_mark(ALArena *a, ALMark *out) {
  (void)a;
  (void)out;
  return AL_BAD;
}
int al_arena_rollback(ALArena *a, ALMark mark) {
  (void)a;
  (void)mark;
  return AL_BAD;
}
int al_arena_commit(ALArena *a, ALMark mark) {
  (void)a;
  (void)mark;
  return AL_BAD;
}
int al_arena_release(ALArena *a, ALHandle h) {
  (void)a;
  (void)h;
  return AL_BAD;
}
int al_arena_reset(ALArena *a) {
  (void)a;
  return AL_BAD;
}
int al_buddy_init(ALBuddy *b) {
  (void)b;
  return AL_BAD;
}
int al_buddy_check(const ALBuddy *b) {
  (void)b;
  return AL_BAD;
}
int al_buddy_take(ALBuddy *b, size_t n, ALHandle *out, size_t *offset,
                  size_t *charged) {
  (void)b;
  (void)n;
  (void)out;
  (void)offset;
  (void)charged;
  return AL_BAD;
}
int al_buddy_release(ALBuddy *b, ALHandle h) {
  (void)b;
  (void)h;
  return AL_BAD;
}
int al_stable_init(ALStable *s, ALHeap *heap) {
  (void)s;
  (void)heap;
  return AL_BAD;
}
int al_stable_take(ALStable *s, size_t n, ALStableHandle *out) {
  (void)s;
  (void)n;
  (void)out;
  return AL_BAD;
}
int al_stable_view(ALStable *s, ALStableHandle h, unsigned char **out,
                   size_t *n) {
  (void)s;
  (void)h;
  (void)out;
  (void)n;
  return AL_BAD;
}
int al_stable_resize(ALStable *s, ALStableHandle h, size_t n, int fail) {
  (void)s;
  (void)h;
  (void)n;
  (void)fail;
  return AL_BAD;
}
int al_stable_release(ALStable *s, ALStableHandle h) {
  (void)s;
  (void)h;
  return AL_BAD;
}
int al_extent_choose(ALExtent *a, size_t *count, size_t bytes, size_t minimum,
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
int al_extent_coalesce(ALExtent *a, size_t *count, size_t capacity) {
  (void)a;
  (void)count;
  (void)capacity;
  return AL_BAD;
}
int al_heap_api(ALHeap *h, ALAllocator *out) {
  (void)h;
  (void)out;
  return AL_BAD;
}
int al_api_allocate(const ALAllocator *a, size_t n, size_t align,
                    ALHandle *out) {
  (void)a;
  (void)n;
  (void)align;
  (void)out;
  return AL_BAD;
}
int al_heap_check(const ALHeap *h) {
  (void)h;
  return AL_BAD;
}
int al_heap_init(ALHeap *h, unsigned n, unsigned best, unsigned deferred,
                 unsigned instrument) {
  (void)h;
  (void)n;
  (void)best;
  (void)deferred;
  (void)instrument;
  return AL_BAD;
}
int al_heap_take(ALHeap *h, size_t n, size_t alignment, ALHandle *out) {
  (void)h;
  (void)n;
  (void)alignment;
  (void)out;
  return AL_BAD;
}
int al_heap_view(ALHeap *h, ALHandle handle, unsigned char **out, size_t *n) {
  (void)h;
  (void)handle;
  (void)out;
  (void)n;
  return AL_BAD;
}
int al_heap_release(ALHeap *h, ALHandle handle) {
  (void)h;
  (void)handle;
  return AL_BAD;
}
int al_heap_collect(ALHeap *h) {
  (void)h;
  return AL_BAD;
}
int al_heap_stats(const ALHeap *h, size_t *total, size_t *largest) {
  (void)h;
  (void)total;
  (void)largest;
  return AL_BAD;
}
int al_heap_resize_copy(ALHeap *h, ALHandle handle, size_t n, int fail_acquire,
                        ALCopy copier, void *context, ALHandle *out) {
  (void)h;
  (void)handle;
  (void)n;
  (void)fail_acquire;
  (void)copier;
  (void)context;
  (void)out;
  return AL_BAD;
}
int al_heap_resize(ALHeap *h, ALHandle handle, size_t n, int fail_acquire,
                   size_t fail_after, ALHandle *out) {
  (void)h;
  (void)handle;
  (void)n;
  (void)fail_acquire;
  (void)fail_after;
  (void)out;
  return AL_BAD;
}
void al_pool_init(ALPool *p) {
  (void)p;
  return;
}
int al_pool_take(ALPool *p, int32_t value, ALPoolHandle *out) {
  (void)p;
  (void)value;
  (void)out;
  return AL_BAD;
}
int al_pool_get(const ALPool *p, ALPoolHandle h, int32_t *out) {
  (void)p;
  (void)h;
  (void)out;
  return AL_BAD;
}
int al_pool_release(ALPool *p, ALPoolHandle h) {
  (void)p;
  (void)h;
  return AL_BAD;
}
int al_classes_init(ALClasses *c) {
  (void)c;
  return AL_BAD;
}
int al_classes_take(ALClasses *c, size_t n,
                    int (*init)(void *, unsigned char *, size_t),
                    ALCleanup destroy, void *ctx, ALHandle *out) {
  (void)c;
  (void)n;
  (void)init;
  (void)destroy;
  (void)ctx;
  (void)out;
  return AL_BAD;
}
int al_classes_release(ALClasses *c, ALHandle h) {
  (void)c;
  (void)h;
  return AL_BAD;
}
int al_classes_stats(const ALClasses *c, size_t *payload, size_t *charged,
                     size_t *retained) {
  (void)c;
  (void)payload;
  (void)charged;
  (void)retained;
  return AL_BAD;
}
