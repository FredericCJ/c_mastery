#define _GNU_SOURCE
#include "allocation.h"
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "ASSERT backends line %d: %s\n", __LINE__, #x);          \
      return 1;                                                                \
    }                                                                          \
  } while (0)
struct al_object {
  int value;
  double measure;
};
struct al_shared {
  struct al_heap heap;
  pthread_mutex_t lock;
  unsigned successes;
};
struct al_worker {
  struct al_shared *shared;
  unsigned iterations;
  int status;
  uint64_t lock_ns;
};
static uint64_t now(void) {
  struct timespec t;
  if (clock_gettime(CLOCK_MONOTONIC, &t))
    abort();
  return (uint64_t)t.tv_sec * 1000000000u + (uint64_t)t.tv_nsec;
}
static void *worker(void *argument) {
  struct al_worker *w = argument;
  for (unsigned i = 0; i < w->iterations; ++i) {
    struct al_handle h;
    uint64_t start = now();
    if (pthread_mutex_lock(&w->shared->lock))
      abort();
    w->lock_ns += now() - start;
    int status = al_heap_take(&w->shared->heap, 4, 8, &h);
    if (!status) {
      unsigned char *p;
      size_t n;
      status = al_heap_view(&w->shared->heap, h, &p, &n);
      if (!status) {
        p[0] = (unsigned char)i;
        status = al_heap_release(&w->shared->heap, h);
        ++w->shared->successes;
      }
    }
    if (status)
      w->status = status;
    if (pthread_mutex_unlock(&w->shared->lock))
      abort();
  }
  return 0;
}
struct al_publication {
  _Atomic unsigned ready, done;
  struct al_object value;
};
static void *reader(void *argument) {
  struct al_publication *p = argument;
  while (!atomic_load_explicit(&p->ready, memory_order_acquire))
    sched_yield();
  if (p->value.value != 81 || p->value.measure != 2.5)
    abort();
  atomic_store_explicit(&p->done, 1, memory_order_release);
  return 0;
}
struct al_private {
  _Alignas(8) unsigned char data[24];
  struct al_arena arena;
  struct al_handle held;
  int status;
};
static void *private_worker(void *argument) {
  struct al_private *p = argument;
  p->status = al_arena_init(&p->arena, p->data, 24, 8);
  if (!p->status)
    p->status = al_arena_take(&p->arena, 24, 8, 0, 0, &p->held);
  return 0;
}
int main(void) {
  _Alignas(16) unsigned char storage[128];
  void *allocated = malloc(128);
  void *mapped =
      mmap(0, 128, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  CHECK(allocated && mapped != MAP_FAILED);
  struct al_arena a[3];
  unsigned char *bases[] = {storage, allocated, mapped};
  size_t offsets[3];
  for (unsigned i = 0; i < 3; ++i) {
    struct al_handle h;
    unsigned char *p;
    size_t n;
    CHECK(al_arena_init(&a[i], bases[i], 128, 8) == AL_OK);
    CHECK(al_arena_take(&a[i], 3, 1, 0, 0, &h) == AL_OK);
    CHECK(al_arena_take(&a[i], 5, 8, 0, 0, &h) == AL_OK);
    CHECK(al_arena_view(&a[i], h, &p, &n) == AL_OK);
    offsets[i] = (size_t)(p - bases[i]);
    for (size_t j = 0; j < n; ++j)
      p[j] = (unsigned char)j;
    CHECK(al_arena_reset(&a[i]) == AL_OK);
  }
  CHECK(offsets[0] == 8 && offsets[1] == 8 && offsets[2] == 8);
  /* Genuine allocated storage establishes Object's effective type through
   * stores. */
  struct al_handle object_handle;
  unsigned char *bytes;
  size_t n;
  CHECK(al_arena_init(&a[1], allocated, 128, _Alignof(max_align_t)) == AL_OK);
  CHECK(al_arena_take(&a[1], sizeof(struct al_object), _Alignof(struct al_object), 0, 0,
                      &object_handle) == AL_OK);
  CHECK(al_arena_view(&a[1], object_handle, &bytes, &n) == AL_OK);
  struct al_object *object = (struct al_object *)(void *)bytes;
  *object = (struct al_object){17, 1.5};
  CHECK(object->value == 17 && object->measure == 1.5);
  CHECK(al_arena_reset(&a[1]) == AL_OK);
  free(allocated);
  CHECK(!munmap(mapped, 128));
  struct al_object typed[3] = {{7, 2.0}};
  CHECK(typed[0].value == 7); /* Actual typed array, distinct backend. */
  struct al_private private_regions[2] = {0};
  pthread_t threads[2];
  for (unsigned i = 0; i < 2; ++i)
    CHECK(!pthread_create(&threads[i], 0, private_worker, &private_regions[i]));
  for (unsigned i = 0; i < 2; ++i)
    CHECK(!pthread_join(threads[i], 0) && !private_regions[i].status);
  struct al_handle extra;
  CHECK(al_arena_reset(&private_regions[1].arena) == AL_OK);
  CHECK(al_arena_take(&private_regions[0].arena, 1, 1, 0, 0, &extra) ==
        AL_FULL);
  CHECK(al_arena_view(&private_regions[1].arena, private_regions[0].held,
                      &bytes, &n) == AL_STALE);
  /* Owner lives in main; worker termination does not end region ownership. */
  CHECK(al_arena_view(&private_regions[0].arena, private_regions[0].held,
                      &bytes, &n) == AL_OK &&
        n == 24);
  CHECK(al_arena_reset(&private_regions[0].arena) == AL_OK);
  struct al_shared shared = {0};
  CHECK(!pthread_mutex_init(&shared.lock, 0));
  CHECK(al_heap_init(&shared.heap, 256, 0, 0, 1) == AL_OK);
  struct al_worker workers[2] = {{&shared, 1000, 0, 0}, {&shared, 1000, 0, 0}};
  for (unsigned i = 0; i < 2; ++i)
    CHECK(!pthread_create(&threads[i], 0, worker, &workers[i]));
  for (unsigned i = 0; i < 2; ++i)
    CHECK(!pthread_join(threads[i], 0) && !workers[i].status);
  CHECK(shared.successes == 2000 && al_heap_check(&shared.heap) == AL_OK);
  CHECK(!pthread_mutex_destroy(&shared.lock));
  struct al_publication publication = {0};
  CHECK(!pthread_create(&threads[0], 0, reader, &publication));
  publication.value = (struct al_object){81, 2.5};
  atomic_store_explicit(&publication.ready, 1, memory_order_release);
  CHECK(!pthread_join(threads[0], 0));
  CHECK(atomic_load_explicit(&publication.done, memory_order_acquire));
  printf("{\"status\":\"PASS\",\"same_policy_offsets\":[8,8,8],\"shared_"
         "operations\":2000,\"lock_acquisition_ns\":[%llu,%llu],\"stranded_"
         "capacity\":24,\"worker_exit_owner_retained\":true,\"publication\":"
         "\"release-acquire plus join\"}\n",
         (unsigned long long)workers[0].lock_ns,
         (unsigned long long)workers[1].lock_ns);
  return 0;
}
