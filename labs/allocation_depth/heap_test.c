#include "allocation.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                        \
    do                                                                  \
    {                                                                   \
        if (!(x))                                                       \
        {                                                               \
            fprintf(stderr, "ASSERT heap line %d: %s\n", __LINE__, #x); \
            return 1;                                                   \
        }                                                               \
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
    SMALL_HEAP_BYTES = 64,
    FULL_HEAP_BYTES = AL_HEAP_BYTES,
    LIVE_SLOTS = 12, /* objects the random workload keeps live */
    WORKLOAD_STEPS = 256, /* operations per seed */
    WORKLOAD_SEEDS = 96,
    MAX_REQUEST = 49,
    PATTERN_STRIDE = 17
};

/* Linear congruential generator (Numerical Recipes constants), seedable. */
static unsigned next(unsigned *s)
{
    *s = *s * 1664525u + 1013904223u;
    return *s;
}

/* Byte `k` of object `index` is always pattern_byte(index, k). */
static unsigned char pattern_byte(unsigned index, unsigned k)
{
    return (unsigned char)(index * PATTERN_STRIDE + k);
}

/* Moving resize preserves contents; failures leave the heap bit-identical. */
static int check_resize(void)
{
    struct al_heap h;
    struct al_heap copy;
    struct al_handle a;
    struct al_handle b;
    struct al_handle c;
    struct al_handle out = {0};
    unsigned char *p;
    size_t n;
    size_t total;
    size_t big;
    CHECK(al_heap_init(&h, SMALL_HEAP_BYTES, 0, 0, 1) == AL_OK);
    CHECK(al_heap_take(&h, 4, 8, &a) == AL_OK);
    CHECK(al_heap_view(&h, a, &p, &n) == AL_OK && n == 4);
    p[0] = 10;
    p[1] = 20;
    p[2] = 30;
    p[3] = 40;
    CHECK(al_heap_take(&h, 4, 8, &b) == AL_OK);
    copy = h;
    CHECK(al_heap_resize(&h, a, 20, 1, SIZE_MAX, &out) == AL_FULL &&
          memcmp(&h, &copy, sizeof h) == 0);
    CHECK(al_heap_resize(&h, a, 20, 0, 2, &out) == AL_CLEANUP &&
          memcmp(&h, &copy, sizeof h) == 0);
    CHECK(al_heap_resize(&h, a, 20, 0, SIZE_MAX, &c) == AL_OK);
    CHECK(al_heap_view(&h, a, &p, &n) == AL_STALE);
    CHECK(al_heap_view(&h, c, &p, &n) == AL_OK && n == 20 && p[0] == 10 &&
          p[3] == 40);
    CHECK(al_heap_release(&h, b) == AL_OK);
    CHECK(al_heap_release(&h, c) == AL_OK);
    CHECK(al_heap_stats(&h, &total, &big) == AL_OK &&
          total == SMALL_HEAP_BYTES && big == SMALL_HEAP_BYTES);
    CHECK(al_heap_take(&h, 4, 1, &a) == AL_OK);
    CHECK(al_heap_resize(&h, a, 25, 0, SIZE_MAX, &b) == AL_OK);
    CHECK(al_heap_resize(&h, b, 1, 0, SIZE_MAX, &c) == AL_OK);
    CHECK(al_heap_check(&h) == AL_OK);
    CHECK(al_heap_release(&h, c) == AL_OK);
    return 0;
}

/* Every payload size 1..52 fits the 64-byte heap and frees back to one block. */
static int check_size_sweep(void)
{
    struct al_heap h;
    struct al_handle a;
    for (unsigned size = 1; size <= 52; ++size)
    {
        CHECK(al_heap_init(&h, SMALL_HEAP_BYTES, 0, 0, 1) == AL_OK);
        CHECK(al_heap_take(&h, size, 8, &a) == AL_OK);
        CHECK(al_heap_check(&h) == AL_OK);
        CHECK(al_heap_release(&h, a) == AL_OK && h.free_count == 1);
    }
    return 0;
}

/* Deferred coalescing keeps free blocks apart until an explicit collect. */
static int check_deferred_collect(struct al_heap *h)
{
    struct al_handle a;
    struct al_handle b;
    CHECK(al_heap_init(h, SMALL_HEAP_BYTES, 0, 1, 1) == AL_OK);
    CHECK(al_heap_take(h, 4, 8, &a) == AL_OK);
    CHECK(al_heap_take(h, 4, 8, &b) == AL_OK);
    CHECK(al_heap_release(h, a) == AL_OK);
    CHECK(al_heap_release(h, b) == AL_OK && h->free_count == 3);
    CHECK(al_heap_collect(h) == AL_OK && h->free_count == 1);
    return 0;
}

/* The checker must parse the physical partition, not trust the free index. */
static int check_corruption_detected(const struct al_heap *good)
{
    struct al_heap h = *good;
    h.bytes[60] = 8;
    CHECK(al_heap_check(&h) == AL_CORRUPT);
    h = *good;
    h.free_offsets[1] = 0;
    h.free_count = 2;
    CHECK(al_heap_check(&h) == AL_CORRUPT);
    h = *good;
    h.bytes[0] = 255;
    CHECK(al_heap_check(&h) == AL_CORRUPT);
    h = *good;
    h.capacity = 15;
    CHECK(al_heap_check(&h) == AL_CORRUPT);
    return 0;
}

/* Independent record of what the workload believes is live. */
struct workload
{
    struct al_heap heap;
    struct al_handle live[LIVE_SLOTS];
    unsigned lengths[LIVE_SLOTS]; /* 0 = slot empty */
    unsigned state; /* generator state */
};

static void fill_pattern(unsigned char *p, unsigned index, unsigned from,
                         unsigned to)
{
    for (unsigned k = from; k < to; ++k)
    {
        p[k] = pattern_byte(index, k);
    }
}

static int workload_take(struct workload *w, unsigned index, unsigned request)
{
    struct al_heap before = w->heap;
    struct al_handle out = {0};
    unsigned char *p;
    size_t n;
    unsigned alignment = 1u << (next(&w->state) % 4);
    int result = al_heap_take(&w->heap, request, alignment, &out);
    CHECK(result == AL_OK || result == AL_FULL);
    if (result != AL_OK)
    {
        CHECK(memcmp(&w->heap, &before, sizeof before) == 0);
        return 0;
    }
    w->live[index] = out;
    w->lengths[index] = request;
    CHECK(al_heap_view(&w->heap, out, &p, &n) == AL_OK);
    fill_pattern(p, index, 0, (unsigned)n);
    return 0;
}

static int workload_release(struct workload *w, unsigned index)
{
    CHECK(al_heap_release(&w->heap, w->live[index]) == AL_OK);
    CHECK(al_heap_release(&w->heap, w->live[index]) == AL_STALE);
    w->lengths[index] = 0;
    return 0;
}

/*
 * `op` 1 refuses acquisition and fails any copy; `op` 2 allows both. Either
 * way a rejected resize must leave the heap bit-identical.
 */
static int workload_resize(struct workload *w, unsigned index, unsigned request,
                           unsigned op)
{
    struct al_heap before = w->heap;
    struct al_handle out = {0};
    unsigned char *p;
    size_t n;
    int result = al_heap_resize(&w->heap, w->live[index], request, op == 1,
                                op == 1 ? 0 : SIZE_MAX, &out);
    CHECK(result == AL_OK || result == AL_FULL || result == AL_CLEANUP);
    if (result != AL_OK)
    {
        CHECK(memcmp(&w->heap, &before, sizeof before) == 0);
        return 0;
    }
    CHECK(al_heap_view(&w->heap, w->live[index], &p, &n) == AL_STALE);
    CHECK(al_heap_view(&w->heap, out, &p, &n) == AL_OK);
    unsigned retained =
        w->lengths[index] < request ? w->lengths[index] : request;
    for (unsigned k = 0; k < retained; ++k)
    {
        CHECK(p[k] == pattern_byte(index, k));
    }
    fill_pattern(p, index, retained, (unsigned)n);
    w->live[index] = out;
    w->lengths[index] = request;
    return 0;
}

/* After every step: metadata valid, payloads intact, extents disjoint. */
static int workload_verify(struct workload *w)
{
    unsigned char occupied[AL_HEAP_BYTES] = {0};
    size_t payload = 0;
    CHECK(al_heap_check(&w->heap) == AL_OK);
    for (unsigned j = 0; j < LIVE_SLOTS; ++j)
    {
        unsigned char *p;
        size_t n;
        if (!w->lengths[j])
        {
            continue;
        }
        CHECK(al_heap_view(&w->heap, w->live[j], &p, &n) == AL_OK &&
              n == w->lengths[j]);
        size_t off = (size_t)(p - w->heap.bytes);
        CHECK(off % 8 == 0 && off + n <= w->heap.capacity);
        for (unsigned k = 0; k < n; ++k)
        {
            CHECK(!occupied[off + k]);
            occupied[off + k] = 1;
            CHECK(p[k] == pattern_byte(j, k));
        }
        payload += n;
    }
    CHECK(w->heap.payload == payload);
    return 0;
}

static int run_workload(unsigned policy, unsigned seed, unsigned *operations)
{
    struct workload w = {0};
    size_t total;
    size_t big;
    CHECK(al_heap_init(&w.heap, FULL_HEAP_BYTES, policy, 0, 1) == AL_OK);
    w.state = seed;
    for (unsigned step = 0; step < WORKLOAD_STEPS; ++step)
    {
        unsigned index = next(&w.state) % LIVE_SLOTS;
        unsigned op = next(&w.state) % 3;
        unsigned request = 1 + next(&w.state) % MAX_REQUEST;
        if (!w.lengths[index])
        {
            RUN_PHASE(workload_take(&w, index, request));
        }
        else if (op == 0)
        {
            RUN_PHASE(workload_release(&w, index));
        }
        else
        {
            RUN_PHASE(workload_resize(&w, index, request, op));
        }
        RUN_PHASE(workload_verify(&w));
        ++*operations;
    }
    for (unsigned j = 0; j < LIVE_SLOTS; ++j)
    {
        if (w.lengths[j])
        {
            CHECK(al_heap_release(&w.heap, w.live[j]) == AL_OK);
        }
    }
    CHECK(al_heap_stats(&w.heap, &total, &big) == AL_OK &&
          total == FULL_HEAP_BYTES && big == FULL_HEAP_BYTES);
    return 0;
}

int main(void)
{
    struct al_heap h;
    unsigned operations = 0;
    RUN_PHASE(check_resize());
    RUN_PHASE(check_size_sweep());
    RUN_PHASE(check_deferred_collect(&h));
    RUN_PHASE(check_corruption_detected(&h));
    for (unsigned policy = 0; policy < 2; ++policy)
    {
        for (unsigned seed = 1; seed <= WORKLOAD_SEEDS; ++seed)
        {
            RUN_PHASE(run_workload(policy, seed, &operations));
        }
    }
    printf("PASS heap operations=%u patterns/nonoverlap/failure/metadata\n",
           operations);
    return 0;
}
