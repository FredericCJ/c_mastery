#include "allocation.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                           \
    do                                                                     \
    {                                                                      \
        if (!(x))                                                          \
        {                                                                  \
            fprintf(stderr, "ASSERT extents line %d: %s\n", __LINE__, #x); \
            return 1;                                                      \
        }                                                                  \
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
    MIN_REMAINDER = 8, /* smallest reusable remainder in the choose tests */
    COPY_FAIL_AFTER = 2, /* failing_copy gives up at this byte index */
    CLIENT_OBJECTS = 8, /* acquisitions in the failure-boundary client */
    PERMUTATIONS = 6 /* orderings of three release events */
};

struct al_copy
{
    unsigned acquired;
    unsigned cleaned;
};

/* Copy callback that fails at byte COPY_FAIL_AFTER and records its cleanup. */
static int failing_copy(void *context, unsigned char *dst,
                        const unsigned char *src, size_t n)
{
    struct al_copy *c = context;
    ++c->acquired;
    for (size_t i = 0; i < n; ++i)
    {
        if (i == COPY_FAIL_AFTER)
        {
            ++c->cleaned;
            return 1;
        }
        dst[i] = src[i];
    }
    return 0;
}

/* First fit, absorbed remainders (threshold 8), best fit, and no-fit. */
static int check_choose(void)
{
    struct al_extent extents[AL_EXTENT_MAX] = {{0, 12}, {20, 20}};
    size_t count = 2;
    size_t at;
    size_t charge;
    CHECK(al_extent_choose(extents, &count, 10, MIN_REMAINDER, 0, &at,
                           &charge) == AL_OK &&
          at == 0 && charge == 12 && count == 1 && extents[0].offset == 20);
    /* Tail 7 is absorbed (charge 17); tails 8 and 9 stay reusable. */
    for (unsigned tail = 7; tail <= 9; ++tail)
    {
        extents[0] = (struct al_extent){0, 10 + tail};
        count = 1;
        CHECK(al_extent_choose(extents, &count, 10, MIN_REMAINDER, 0, &at,
                               &charge) == AL_OK);
        CHECK(tail == 7
                  ? (count == 0 && charge == 17)
                  : (count == 1 && charge == 10 && extents[0].size == tail));
    }
    extents[0] = (struct al_extent){0, 24};
    extents[1] = (struct al_extent){32, 16};
    extents[2] = (struct al_extent){56, 16};
    count = 3;
    CHECK(al_extent_choose(extents, &count, 10, MIN_REMAINDER, 1, &at,
                           &charge) == AL_OK &&
          at == 32 && charge == 16);
    extents[0] = (struct al_extent){0, 8};
    extents[1] = (struct al_extent){16, 8};
    extents[2] = (struct al_extent){32, 8};
    count = 3;
    CHECK(al_extent_choose(extents, &count, 9, MIN_REMAINDER, 0, &at,
                           &charge) == AL_FULL);
    return 0;
}

/* Three adjacent 8-byte extents released in all 6 orders merge into one. */
static int check_coalesce(unsigned *permutations)
{
    static const unsigned orders[PERMUTATIONS][3] = {
        {0, 1, 2}, {0, 2, 1}, {1, 0, 2}, {1, 2, 0}, {2, 0, 1}, {2, 1, 0}};
    struct al_extent extents[AL_EXTENT_MAX];
    for (unsigned o = 0; o < PERMUTATIONS; ++o)
    {
        size_t count = 0;
        for (unsigned i = 0; i < 3; ++i)
        {
            extents[count++] = (struct al_extent){8 * orders[o][i], 8};
            CHECK(al_extent_coalesce(extents, &count, 24) == AL_OK);
        }
        CHECK(count == 1 && extents[0].size == 24);
        ++*permutations;
    }
    return 0;
}

/* The type-erased interface, the client's copy callback and a moving resize. */
static int check_allocator_api(struct al_heap *heap, struct al_allocator *api)
{
    struct al_heap saved;
    struct al_handle h;
    struct al_handle b;
    struct al_handle out;
    unsigned char *p;
    size_t n;
    CHECK(al_heap_init(heap, 64, 0, 0, 1) == AL_OK &&
          al_heap_api(heap, api) == AL_OK);
    CHECK(al_api_allocate(api, 4, 8, &h) == AL_OK);
    CHECK(al_api_allocate(api, 4, 8, &b) == AL_OK);
    saved = *heap;
    CHECK(al_api_allocate(api, 0, 8, &out) == AL_BAD &&
          memcmp(heap, &saved, sizeof *heap) == 0);
    CHECK(al_api_allocate(api, 4, 16, &out) == AL_BAD);
    struct al_copy copy = {0};
    CHECK(al_heap_resize_copy(heap, h, 20, 0, failing_copy, &copy, &out) ==
              AL_CLEANUP &&
          memcmp(heap, &saved, sizeof *heap) == 0 && copy.acquired == 1 &&
          copy.cleaned == 1);
    CHECK(api->release(api->owner, b) == AL_OK);
    /* A stable client token indexes this descriptor, never a payload address. */
    CHECK(api->resize(api->owner, h, 20, &out) == AL_OK);
    struct al_handle descriptor = out;
    CHECK(api->resize(api->owner, descriptor, 30, &out) == AL_OK);
    descriptor = out;
    CHECK(al_heap_view(heap, descriptor, &p, &n) == AL_OK && n == 30);
    CHECK(api->release(api->owner, descriptor) == AL_OK);
    return 0;
}

/* Stable handles survive a moving resize; foreign tables see them as stale. */
static int check_stable_handles(struct al_heap *heap)
{
    struct al_heap saved;
    struct al_stable stable;
    struct al_stable foreign;
    struct al_stable_handle stable_h;
    struct al_stable_handle stable_b;
    unsigned char *p;
    size_t n;
    CHECK(al_heap_init(heap, 64, 0, 0, 1) == AL_OK);
    CHECK(al_stable_init(&stable, heap) == AL_OK &&
          al_stable_init(&foreign, heap) == AL_OK);
    CHECK(al_stable_take(&stable, 4, &stable_h) == AL_OK &&
          al_stable_take(&stable, 4, &stable_b) == AL_OK);
    CHECK(al_stable_view(&stable, stable_h, &p, &n) == AL_OK);
    p[0] = 93;
    struct al_stable stable_saved = stable;
    saved = *heap;
    CHECK(al_stable_resize(&stable, stable_h, 20, 1) == AL_FULL &&
          memcmp(heap, &saved, sizeof *heap) == 0 &&
          memcmp(&stable, &stable_saved, sizeof stable) == 0);
    CHECK(al_stable_resize(&stable, stable_h, 20, 0) == AL_OK);
    CHECK(al_stable_view(&stable, stable_h, &p, &n) == AL_OK && n == 20 &&
          p[0] == 93);
    CHECK(al_stable_view(&foreign, stable_h, &p, &n) == AL_STALE);
    CHECK(al_stable_release(&stable, stable_h) == AL_OK &&
          al_stable_view(&stable, stable_h, &p, &n) == AL_STALE);
    CHECK(al_stable_release(&stable, stable_b) == AL_OK);
    return 0;
}

/*
 * Every acquisition in this eight-object client construction is a failure
 * boundary. The studied allocator remains local; the harness gate fails
 * before delegate invocation and the client releases its completed prefix.
 * `fail` 9 means no acquisition fails.
 */
static int check_failure_boundaries(struct al_heap *heap,
                                    const struct al_allocator *api)
{
    for (unsigned fail = 1; fail <= CLIENT_OBJECTS + 1; ++fail)
    {
        struct al_handle completed[CLIENT_OBJECTS];
        struct al_handle out;
        unsigned used = 0;
        CHECK(al_heap_init(heap, AL_HEAP_BYTES, 0, 0, 1) == AL_OK);
        for (unsigned ordinal = 1; ordinal <= CLIENT_OBJECTS; ++ordinal)
        {
            struct al_heap before = *heap;
            int status =
                ordinal == fail ? AL_FULL : al_api_allocate(api, 4, 8, &out);
            if (status)
            {
                CHECK(status == AL_FULL &&
                      memcmp(&before, heap, sizeof before) == 0);
                break;
            }
            completed[used++] = out;
        }
        CHECK(used == (fail <= CLIENT_OBJECTS ? fail - 1 : CLIENT_OBJECTS));
        while (used)
        {
            CHECK(api->release(api->owner, completed[--used]) == AL_OK);
        }
        CHECK(heap->payload == 0 && heap->free_count == 1);
    }
    return 0;
}

int main(void)
{
    struct al_heap heap;
    struct al_allocator api;
    unsigned permutations = 0;
    RUN_PHASE(check_choose());
    RUN_PHASE(check_coalesce(&permutations));
    RUN_PHASE(check_allocator_api(&heap, &api));
    RUN_PHASE(check_stable_handles(&heap));
    RUN_PHASE(check_failure_boundaries(&heap, &api));
    printf("PASS extents release-orders=%u "
           "exact8-threshold/callbacks/copy-cleanup\n",
           permutations);
    return 0;
}
