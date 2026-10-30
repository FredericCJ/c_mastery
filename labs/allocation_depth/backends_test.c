#define _GNU_SOURCE
#include "allocation.h"
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#define CHECK(x)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "ASSERT backends line %d: %s\n", __LINE__, #x); \
            return 1;                                                       \
        }                                                                   \
    } while (0)

/* Each phase returns 0 on success, 1 after printing its failed CHECK. */
#define RUN_PHASE(phase) \
    do                   \
    {                    \
        if (phase)       \
        {                \
            return 1;    \
        }                \
    } while (0)

enum
{
    REGION_BYTES = 128, /* each arena backend in the first phase */
    PRIVATE_REGION_BYTES = 24, /* per-thread private arena */
    BACKENDS = 3, /* declared array, malloc, mmap */
    THREADS = 2,
    WORKER_ITERATIONS = 1000,
    PUBLISHED_VALUE = 81
};

static const uint64_t NS_PER_SECOND = UINT64_C(1000000000);
static const double PUBLISHED_MEASURE = 2.5;

struct al_object
{
    int value;
    double measure;
};

/* Heap shared by workers; every access to `heap` holds `lock`. */
struct al_shared
{
    struct al_heap heap;
    pthread_mutex_t lock;
    unsigned successes;
};

struct al_worker
{
    struct al_shared *shared;
    unsigned iterations;
    int status; /* last nonzero allocator status, 0 if none */
    uint64_t lock_ns; /* time spent waiting for the lock */
};

static uint64_t now(void)
{
    struct timespec t;
    if (clock_gettime(CLOCK_MONOTONIC, &t))
    {
        abort();
    }
    return (uint64_t)t.tv_sec * NS_PER_SECOND + (uint64_t)t.tv_nsec;
}

/* Take, write, release one 4-byte object per iteration, under the lock. */
static void *worker(void *argument)
{
    struct al_worker *w = argument;
    for (unsigned i = 0; i < w->iterations; ++i)
    {
        struct al_handle h;
        uint64_t start = now();
        if (pthread_mutex_lock(&w->shared->lock))
        {
            abort();
        }
        w->lock_ns += now() - start;
        int status = al_heap_take(&w->shared->heap, 4, 8, &h);
        if (!status)
        {
            unsigned char *p;
            size_t n;
            status = al_heap_view(&w->shared->heap, h, &p, &n);
            if (!status)
            {
                p[0] = (unsigned char)i;
                status = al_heap_release(&w->shared->heap, h);
                ++w->shared->successes;
            }
        }
        if (status)
        {
            w->status = status;
        }
        if (pthread_mutex_unlock(&w->shared->lock))
        {
            abort();
        }
    }
    return 0;
}

/* `value` is written before the release store of `ready` and read after the
   acquire load, so the plain object needs no atomic of its own. */
struct al_publication
{
    _Atomic unsigned ready;
    _Atomic unsigned done;
    struct al_object value;
};

static void *reader(void *argument)
{
    struct al_publication *p = argument;
    while (!atomic_load_explicit(&p->ready, memory_order_acquire))
    {
        sched_yield();
    }
    if (p->value.value != PUBLISHED_VALUE ||
        p->value.measure != PUBLISHED_MEASURE)
    {
        abort();
    }
    atomic_store_explicit(&p->done, 1, memory_order_release);
    return 0;
}

/* A region owned by main, used by exactly one worker thread at a time. */
struct al_private
{
    _Alignas(8) unsigned char data[PRIVATE_REGION_BYTES];
    struct al_arena arena;
    struct al_handle held;
    int status;
};

static void *private_worker(void *argument)
{
    struct al_private *p = argument;
    p->status = al_arena_init(&p->arena, p->data, PRIVATE_REGION_BYTES, 8);
    if (!p->status)
    {
        p->status =
            al_arena_take(&p->arena, PRIVATE_REGION_BYTES, 8, 0, 0, &p->held);
    }
    return 0;
}

/*
 * The same policy (arena over caller bytes) on three backings must place the
 * same relative offsets; then genuine allocated storage gets a typed object.
 * The malloc and mmap regions are released here.
 */
static int check_arena_backends(void)
{
    _Alignas(16) unsigned char storage[REGION_BYTES];
    void *allocated = malloc(REGION_BYTES);
    void *mapped = mmap(0, REGION_BYTES, PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    CHECK(allocated && mapped != MAP_FAILED);
    struct al_arena a[BACKENDS];
    unsigned char *bases[BACKENDS] = {storage, allocated, mapped};
    size_t offsets[BACKENDS];
    for (unsigned i = 0; i < BACKENDS; ++i)
    {
        struct al_handle h;
        unsigned char *p;
        size_t n;
        CHECK(al_arena_init(&a[i], bases[i], REGION_BYTES, 8) == AL_OK);
        CHECK(al_arena_take(&a[i], 3, 1, 0, 0, &h) == AL_OK);
        CHECK(al_arena_take(&a[i], 5, 8, 0, 0, &h) == AL_OK);
        CHECK(al_arena_view(&a[i], h, &p, &n) == AL_OK);
        offsets[i] = (size_t)(p - bases[i]);
        for (size_t j = 0; j < n; ++j)
        {
            p[j] = (unsigned char)j;
        }
        CHECK(al_arena_reset(&a[i]) == AL_OK);
    }
    CHECK(offsets[0] == 8 && offsets[1] == 8 && offsets[2] == 8);
    /* Genuine allocated storage establishes Object's effective type through
     * stores. */
    struct al_handle object_handle;
    unsigned char *bytes;
    size_t n;
    CHECK(al_arena_init(&a[1], allocated, REGION_BYTES,
                        _Alignof(max_align_t)) == AL_OK);
    CHECK(al_arena_take(&a[1], sizeof(struct al_object),
                        _Alignof(struct al_object), 0, 0,
                        &object_handle) == AL_OK);
    CHECK(al_arena_view(&a[1], object_handle, &bytes, &n) == AL_OK);
    struct al_object *object = (struct al_object *)(void *)bytes;
    *object = (struct al_object){17, 1.5};
    CHECK(object->value == 17 && object->measure == 1.5);
    CHECK(al_arena_reset(&a[1]) == AL_OK);
    free(allocated);
    CHECK(!munmap(mapped, REGION_BYTES));
    struct al_object typed[3] = {{7, 2.0}};
    CHECK(typed[0].value == 7); /* Actual typed array, distinct backend. */
    return 0;
}

/* Worker threads fill private regions; ownership stays with main afterwards. */
static int check_private_regions(void)
{
    struct al_private regions[THREADS] = {0};
    pthread_t threads[THREADS];
    unsigned char *bytes;
    size_t n;
    for (unsigned i = 0; i < THREADS; ++i)
    {
        CHECK(!pthread_create(&threads[i], 0, private_worker, &regions[i]));
    }
    for (unsigned i = 0; i < THREADS; ++i)
    {
        CHECK(!pthread_join(threads[i], 0) && !regions[i].status);
    }
    struct al_handle extra;
    CHECK(al_arena_reset(&regions[1].arena) == AL_OK);
    CHECK(al_arena_take(&regions[0].arena, 1, 1, 0, 0, &extra) == AL_FULL);
    CHECK(al_arena_view(&regions[1].arena, regions[0].held, &bytes, &n) ==
          AL_STALE);
    /* Owner lives in main; worker termination does not end region ownership. */
    CHECK(al_arena_view(&regions[0].arena, regions[0].held, &bytes, &n) ==
              AL_OK &&
          n == PRIVATE_REGION_BYTES);
    CHECK(al_arena_reset(&regions[0].arena) == AL_OK);
    return 0;
}

/* Two threads share one heap behind a mutex; reports lock wait per thread. */
static int check_shared_heap(uint64_t lock_ns[THREADS])
{
    struct al_shared shared = {0};
    pthread_t threads[THREADS];
    CHECK(!pthread_mutex_init(&shared.lock, 0));
    CHECK(al_heap_init(&shared.heap, AL_HEAP_BYTES, 0, 0, 1) == AL_OK);
    struct al_worker workers[THREADS] = {{&shared, WORKER_ITERATIONS, 0, 0},
                                         {&shared, WORKER_ITERATIONS, 0, 0}};
    for (unsigned i = 0; i < THREADS; ++i)
    {
        CHECK(!pthread_create(&threads[i], 0, worker, &workers[i]));
    }
    for (unsigned i = 0; i < THREADS; ++i)
    {
        CHECK(!pthread_join(threads[i], 0) && !workers[i].status);
    }
    CHECK(shared.successes == THREADS * WORKER_ITERATIONS &&
          al_heap_check(&shared.heap) == AL_OK);
    CHECK(!pthread_mutex_destroy(&shared.lock));
    for (unsigned i = 0; i < THREADS; ++i)
    {
        lock_ns[i] = workers[i].lock_ns;
    }
    return 0;
}

/* Release/acquire publication of a plain object, then join. */
static int check_publication(void)
{
    struct al_publication publication = {0};
    pthread_t thread;
    CHECK(!pthread_create(&thread, 0, reader, &publication));
    publication.value = (struct al_object){PUBLISHED_VALUE, PUBLISHED_MEASURE};
    atomic_store_explicit(&publication.ready, 1, memory_order_release);
    CHECK(!pthread_join(thread, 0));
    CHECK(atomic_load_explicit(&publication.done, memory_order_acquire));
    return 0;
}

int main(void)
{
    uint64_t lock_ns[THREADS] = {0};
    RUN_PHASE(check_arena_backends());
    RUN_PHASE(check_private_regions());
    RUN_PHASE(check_shared_heap(lock_ns));
    RUN_PHASE(check_publication());
    printf("{\"status\":\"PASS\",\"same_policy_offsets\":[8,8,8],\"shared_"
           "operations\":2000,\"lock_acquisition_ns\":[%llu,%llu],\"stranded_"
           "capacity\":24,\"worker_exit_owner_retained\":true,\"publication\":"
           "\"release-acquire plus join\"}\n",
           (unsigned long long)lock_ns[0], (unsigned long long)lock_ns[1]);
    return 0;
}
