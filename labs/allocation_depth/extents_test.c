#include "allocation.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "ASSERT extents line %d: %s\n", __LINE__, #x);           \
      return 1;                                                                \
    }                                                                          \
  } while (0)
typedef struct {
  unsigned acquired, cleaned;
} Copy;
static int failing_copy(void *context, unsigned char *dst,
                        const unsigned char *src, size_t n) {
  Copy *c = context;
  ++c->acquired;
  for (size_t i = 0; i < n; ++i) {
    if (i == 2) {
      ++c->cleaned;
      return 1;
    }
    dst[i] = src[i];
  }
  return 0;
}
int main(void) {
  ALExtent extents[16] = {{0, 12}, {20, 20}};
  size_t count = 2, at, charge;
  CHECK(al_extent_choose(extents, &count, 10, 8, 0, &at, &charge) == AL_OK &&
        at == 0 && charge == 12 && count == 1 && extents[0].offset == 20);
  for (unsigned tail = 7; tail <= 9; ++tail) {
    extents[0] = (ALExtent){0, 10 + tail};
    count = 1;
    CHECK(al_extent_choose(extents, &count, 10, 8, 0, &at, &charge) == AL_OK);
    CHECK(tail == 7 ? (count == 0 && charge == 17)
                    : (count == 1 && charge == 10 && extents[0].size == tail));
  }
  extents[0] = (ALExtent){0, 24};
  extents[1] = (ALExtent){32, 16};
  extents[2] = (ALExtent){56, 16};
  count = 3;
  CHECK(al_extent_choose(extents, &count, 10, 8, 1, &at, &charge) == AL_OK &&
        at == 32 && charge == 16);
  unsigned permutations = 0;
  for (unsigned a = 0; a < 3; ++a)
    for (unsigned b = 0; b < 3; ++b)
      if (b != a)
        for (unsigned c = 0; c < 3; ++c)
          if (c != a && c != b) {
            unsigned order[] = {a, b, c};
            count = 0;
            for (unsigned i = 0; i < 3; ++i) {
              extents[count++] = (ALExtent){8 * order[i], 8};
              CHECK(al_extent_coalesce(extents, &count, 24) == AL_OK);
            }
            CHECK(count == 1 && extents[0].size == 24);
            ++permutations;
          }
  extents[0] = (ALExtent){0, 8};
  extents[1] = (ALExtent){16, 8};
  extents[2] = (ALExtent){32, 8};
  count = 3;
  CHECK(al_extent_choose(extents, &count, 9, 8, 0, &at, &charge) == AL_FULL);
  ALHeap heap, saved;
  ALAllocator api;
  ALHandle h, b, out;
  CHECK(al_heap_init(&heap, 64, 0, 0, 1) == AL_OK &&
        al_heap_api(&heap, &api) == AL_OK);
  CHECK(al_api_allocate(&api, 4, 8, &h) == AL_OK);
  CHECK(al_api_allocate(&api, 4, 8, &b) == AL_OK);
  saved = heap;
  CHECK(al_api_allocate(&api, 0, 8, &out) == AL_BAD &&
        memcmp(&heap, &saved, sizeof heap) == 0);
  CHECK(al_api_allocate(&api, 4, 16, &out) == AL_BAD);
  Copy copy = {0};
  CHECK(al_heap_resize_copy(&heap, h, 20, 0, failing_copy, &copy, &out) ==
            AL_CLEANUP &&
        memcmp(&heap, &saved, sizeof heap) == 0 && copy.acquired == 1 &&
        copy.cleaned == 1);
  CHECK(api.release(api.owner, b) == AL_OK);
  CHECK(api.resize(api.owner, h, 20, &out) ==
        AL_OK); /* Stable client token indexes this descriptor, never payload
                   address. */
  ALHandle descriptor = out;
  CHECK(api.resize(api.owner, descriptor, 30, &out) == AL_OK);
  descriptor = out;
  unsigned char *p;
  size_t n;
  CHECK(al_heap_view(&heap, descriptor, &p, &n) == AL_OK && n == 30);
  CHECK(api.release(api.owner, descriptor) == AL_OK);
  ALStable stable, foreign;
  ALStableHandle stable_h, stable_b;
  CHECK(al_heap_init(&heap, 64, 0, 0, 1) == AL_OK);
  CHECK(al_stable_init(&stable, &heap) == AL_OK &&
        al_stable_init(&foreign, &heap) == AL_OK);
  CHECK(al_stable_take(&stable, 4, &stable_h) == AL_OK &&
        al_stable_take(&stable, 4, &stable_b) == AL_OK);
  CHECK(al_stable_view(&stable, stable_h, &p, &n) == AL_OK);
  p[0] = 93;
  ALStable stable_saved = stable;
  saved = heap;
  CHECK(al_stable_resize(&stable, stable_h, 20, 1) == AL_FULL &&
        memcmp(&heap, &saved, sizeof heap) == 0 &&
        memcmp(&stable, &stable_saved, sizeof stable) == 0);
  CHECK(al_stable_resize(&stable, stable_h, 20, 0) == AL_OK);
  CHECK(al_stable_view(&stable, stable_h, &p, &n) == AL_OK && n == 20 &&
        p[0] == 93);
  CHECK(al_stable_view(&foreign, stable_h, &p, &n) == AL_STALE);
  CHECK(al_stable_release(&stable, stable_h) == AL_OK &&
        al_stable_view(&stable, stable_h, &p, &n) == AL_STALE);
  CHECK(al_stable_release(&stable, stable_b) == AL_OK);
  /* Every acquisition in this eight-object client construction is a failure
     boundary. The studied allocator remains local; the harness gate fails
     before delegate invocation and the client releases its completed prefix. */
  for (unsigned fail = 1; fail <= 9; ++fail) {
    CHECK(al_heap_init(&heap, 256, 0, 0, 1) == AL_OK);
    ALHandle completed[8];
    unsigned used = 0;
    for (unsigned ordinal = 1; ordinal <= 8; ++ordinal) {
      saved = heap;
      int status =
          ordinal == fail ? AL_FULL : al_api_allocate(&api, 4, 8, &out);
      if (status) {
        CHECK(status == AL_FULL && memcmp(&saved, &heap, sizeof heap) == 0);
        break;
      }
      completed[used++] = out;
    }
    CHECK(used == (fail <= 8 ? fail - 1 : 8));
    while (used)
      CHECK(api.release(api.owner, completed[--used]) == AL_OK);
    CHECK(heap.payload == 0 && heap.free_count == 1);
  }
  printf("PASS extents release-orders=%u "
         "exact8-threshold/callbacks/copy-cleanup\n",
         permutations);
  return 0;
}
