#ifndef ALLOCATION_DEPTH_H
#define ALLOCATION_DEPTH_H
#include "algo.h"
#include <stddef.h>
#include <stdint.h>
enum { AL_OK, AL_BAD, AL_FULL, AL_STALE, AL_CORRUPT, AL_CLEANUP, AL_RETIRED };
typedef int (*ALCleanup)(void *);
typedef struct {
  const void *owner;
  uint32_t id;
} ALHandle;
typedef struct {
  size_t offset, size, before;
  uint32_t id;
  ALCleanup cleanup;
  void *context;
} ALItem;
typedef struct {
  const void *owner;
  uint32_t id;
} ALMark;
typedef struct {
  unsigned char *bytes;
  size_t capacity, alignment, used, payload, peak, epoch_peak;
  ALItem items[16];
  unsigned count, mark_count;
  uint32_t serial;
  struct {
    uint32_t id;
    unsigned count;
    size_t used;
  } marks[8];
} ALArena;
int al_extent(size_t cursor, size_t bytes, size_t alignment, size_t capacity,
              size_t *offset, size_t *end);
int al_arena_init(ALArena *, unsigned char *, size_t, size_t);
int al_arena_take(ALArena *, size_t, size_t, ALCleanup, void *, ALHandle *);
int al_arena_view(ALArena *, ALHandle, unsigned char **, size_t *);
int al_arena_mark(ALArena *, ALMark *);
int al_arena_rollback(ALArena *, ALMark);
int al_arena_commit(ALArena *, ALMark);
int al_arena_release(ALArena *, ALHandle);
int al_arena_reset(ALArena *);
typedef struct {
  _Alignas(8) unsigned char bytes[256];
  unsigned capacity, free_count, free_offsets[16];
  struct {
    uint32_t id;
    unsigned offset;
  } live[16];
  uint32_t serial;
  unsigned policy, deferred, instrument;
  size_t payload, charged, peak_payload, peak_charged, search_steps;
} ALHeap;
int al_heap_init(ALHeap *, unsigned capacity, unsigned best_fit,
                 unsigned deferred, unsigned instrument);
int al_heap_check(const ALHeap *);
int al_heap_take(ALHeap *, size_t, size_t, ALHandle *);
int al_heap_release(ALHeap *, ALHandle);
int al_heap_resize(ALHeap *, ALHandle, size_t, int fail_acquire,
                   size_t fail_copy_after, ALHandle *);
int al_heap_view(ALHeap *, ALHandle, unsigned char **, size_t *);
int al_heap_collect(ALHeap *);
int al_heap_stats(const ALHeap *, size_t *total_free, size_t *largest_free);
typedef struct {
  algo_pool producer;
} ALPool;
typedef struct {
  const ALPool *owner;
  uint64_t raw;
} ALPoolHandle;
void al_pool_init(ALPool *);
int al_pool_take(ALPool *, int32_t, ALPoolHandle *);
int al_pool_get(const ALPool *, ALPoolHandle, int32_t *);
int al_pool_release(ALPool *, ALPoolHandle);
typedef struct {
  unsigned char state[15];
  uint32_t ids[15], serial;
  unsigned steps;
} ALBuddy;
int al_buddy_init(ALBuddy *);
int al_buddy_take(ALBuddy *, size_t, ALHandle *, size_t *offset,
                  size_t *charged);
int al_buddy_release(ALBuddy *, ALHandle);
int al_buddy_check(const ALBuddy *);
typedef struct {
  unsigned char data[32];
  uint32_t id;
  unsigned requested, live;
  ALCleanup cleanup;
  void *context;
} ALClassSlot;
typedef struct {
  ALClassSlot slots[3][4];
  uint32_t serial;
  unsigned retained[3];
} ALClasses;
int al_classes_init(ALClasses *);
int al_classes_take(ALClasses *, size_t,
                    int (*initialize)(void *, unsigned char *, size_t),
                    ALCleanup, void *, ALHandle *);
int al_classes_release(ALClasses *, ALHandle);
int al_classes_stats(const ALClasses *, size_t *payload, size_t *charged,
                     size_t *retained);
typedef struct {
  size_t offset, size;
} ALExtent;
int al_extent_choose(ALExtent *, size_t *count, size_t bytes, size_t minimum,
                     unsigned best_fit, size_t *offset, size_t *charged);
int al_extent_coalesce(ALExtent *, size_t *count, size_t capacity);
typedef struct {
  void *owner;
  size_t max_alignment;
  int (*allocate)(void *, size_t, size_t, ALHandle *);
  int (*release)(void *, ALHandle);
  int (*resize)(void *, ALHandle, size_t, ALHandle *);
} ALAllocator;
int al_heap_api(ALHeap *, ALAllocator *);
int al_api_allocate(const ALAllocator *, size_t, size_t, ALHandle *);
typedef int (*ALCopy)(void *, unsigned char *, const unsigned char *, size_t);
int al_heap_resize_copy(ALHeap *, ALHandle, size_t, int, ALCopy, void *,
                        ALHandle *);
typedef struct {
  ALHeap *heap;
  struct {
    ALHandle inner;
    unsigned generation, state;
  } entries[3];
} ALStable;
typedef struct {
  const ALStable *owner;
  unsigned index, generation;
} ALStableHandle;
int al_stable_init(ALStable *, ALHeap *);
int al_stable_take(ALStable *, size_t, ALStableHandle *);
int al_stable_view(ALStable *, ALStableHandle, unsigned char **, size_t *);
int al_stable_resize(ALStable *, ALStableHandle, size_t, int);
int al_stable_release(ALStable *, ALStableHandle);
#endif
