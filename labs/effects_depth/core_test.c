/*
 * Core regression for the effects components: cache, pool, parser, bounded
 * callbacks, sorting and the logger. Each topic is a function that returns 0
 * when every CHECK holds; main stops at the first failing topic.
 */
#include "effects.h"
#include "logger.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x)                                                     \
    do                                                               \
    {                                                                \
        if (!(x))                                                    \
        {                                                            \
            fprintf(stderr, "EFFECTS_ASSERT:%d:%s\n", __LINE__, #x); \
            return 1;                                                \
        }                                                            \
    } while (0)

/* Allocation counters of the test backend; they are separate quantities. */
struct ef_alloc
{
    unsigned requests;
    unsigned acquisitions;
    unsigned releases;
    unsigned live_bytes;
    unsigned fail;
    unsigned char storage[EF_SCRATCH_BYTES];
};

static unsigned char *take(void *context, size_t bytes)
{
    struct ef_alloc *backend = context;
    ++backend->requests;
    if (bytes != EF_SCRATCH_BYTES || backend->fail || backend->live_bytes)
    {
        return NULL;
    }
    ++backend->acquisitions;
    backend->live_bytes = EF_SCRATCH_BYTES;
    return backend->storage;
}

static void release(void *context, unsigned char *storage)
{
    struct ef_alloc *backend = context;
    if (storage != backend->storage || backend->live_bytes != EF_SCRATCH_BYTES)
    {
        abort();
    }
    ++backend->releases;
    backend->live_bytes = 0;
}

/* Reports success on its third call, counted through *context. */
static int callback(void *context, unsigned char *buffer, size_t buffer_bytes)
{
    unsigned *calls = context;
    if (buffer_bytes != 16)
    {
        abort();
    }
    buffer[0] = 1;
    ++*calls;
    return *calls == 3 ? 1 : 0;
}

static int never_ready(void *context, unsigned char *buffer,
                       size_t buffer_bytes)
{
    (void)context;
    (void)buffer;
    (void)buffer_bytes;
    return 0;
}

static int compare_int(const void *left, const void *right)
{
    int x = *(const int *)left;
    int y = *(const int *)right;
    return (x > y) - (x < y);
}

static int test_cache(void)
{
    struct ef_alloc backend = {0};
    struct ef_cache cache = {{&backend, take, release}, NULL, 1, 0, 0, 0};
    uint32_t value = 99;
    CHECK(ef_checksum(NULL, 0, &value) == EF_OK && value == 0);
    CHECK(ef_checksum(NULL, 1, &value) == EF_INVALID && value == 0);
    CHECK(ef_cached(&cache, (const unsigned char *)"abc", 3, &value) == EF_OK &&
          value == 294);
    CHECK(backend.requests == 1 && backend.acquisitions == 1 &&
          backend.live_bytes == EF_SCRATCH_BYTES);
    for (unsigned i = 0; i < 10; ++i)
    {
        CHECK(ef_cached(&cache, (const unsigned char *)"abc", 3, &value) ==
              EF_OK);
    }
    CHECK(backend.requests == 1);

    const unsigned char *borrow = NULL;
    CHECK(ef_cache_borrow(&cache, &borrow) == EF_OK &&
          borrow == backend.storage);
    CHECK(ef_cache_reset(&cache) == EF_BUSY &&
          backend.live_bytes == EF_SCRATCH_BYTES && cache.generation == 1);
    CHECK(ef_cached(&cache, NULL, 0, &value) == EF_BUSY);
    CHECK(ef_cache_return(&cache) == EF_OK && ef_cache_reset(&cache) == EF_OK &&
          backend.live_bytes == 0 && cache.generation == 2);

    /* A transient allocation failure is retried by the next call. */
    backend.fail = 1;
    value = 77;
    CHECK(ef_cached(&cache, NULL, 0, &value) == EF_NOMEM && value == 77);
    backend.fail = 0;
    CHECK(ef_cached(&cache, NULL, 0, &value) == EF_OK && backend.requests == 3);
    CHECK(ef_cache_reset(&cache) == EF_OK);

    /* A permanent failure latches until reset. */
    cache.permanent = 1;
    backend.fail = 1;
    CHECK(ef_cached(&cache, NULL, 0, &value) == EF_NOMEM);
    backend.fail = 0;
    CHECK(ef_cached(&cache, NULL, 0, &value) == EF_FAILED &&
          backend.requests == 4);
    CHECK(ef_cache_reset(&cache) == EF_OK &&
          ef_cached(&cache, NULL, 0, &value) == EF_OK && backend.requests == 5);
    CHECK(ef_cache_reset(&cache) == EF_OK);

    cache.generation = UINT32_MAX;
    CHECK(ef_cache_reset(&cache) == EF_FULL && cache.generation == UINT32_MAX);
    return 0;
}

static int test_pool(void)
{
    struct ef_pool pool = {{0}, 0, 0};
    unsigned char *slot = NULL;
    for (unsigned i = 0; i < EF_POOL_SLOTS; ++i)
    {
        CHECK(ef_pool_take(&pool, &slot) == EF_OK && slot == &pool.bytes[i]);
    }
    CHECK(ef_pool_take(&pool, &slot) == EF_FULL &&
          pool.requests == EF_POOL_SLOTS + 1);
    CHECK(ef_pool_return(&pool, 7) == EF_OK &&
          ef_pool_take(&pool, &slot) == EF_OK && slot == &pool.bytes[7]);
    CHECK(pool.requests == EF_POOL_SLOTS + 2 && pool.used == 0xFFFFu);
    CHECK(ef_pool_return(&pool, EF_POOL_SLOTS) == EF_INVALID);
    return 0;
}

static int test_parser(void)
{
    struct ef_parser first = {{0}, 0, 7, 1, 0};
    struct ef_parser second = {{0}, 0, 7, 1, 0};
    struct ef_view first_view = {0};
    struct ef_view second_view = {0};
    CHECK(ef_parse(&first, (const unsigned char *)"abc", 3, &first_view) ==
          EF_OK);
    CHECK(ef_parse(&second, (const unsigned char *)"def", 3, &second_view) ==
              EF_OK &&
          memcmp(first_view.data, "ABC", 3) == 0 &&
          memcmp(second_view.data, "DEF", 3) == 0);

    struct ef_view refused = first_view;
    CHECK(ef_parse(&first, (const unsigned char *)"z", 1, &refused) ==
              EF_BUSY &&
          memcmp(first_view.data, "ABC", 3) == 0);
    CHECK(ef_parser_reset(&first, 7) == EF_BUSY && first.cursor == 3);
    CHECK(ef_view_release(&first_view) == EF_OK &&
          ef_view_release(&first_view) == EF_INVALID);

    /* Same seed, same sequence: the first draw is a fixed 32-bit value. */
    uint32_t draw = ef_random(&first);
    CHECK(draw == 1025555898u);
    CHECK(ef_parser_reset(&first, 7) == EF_OK && first.cursor == 0 &&
          ef_random(&first) == draw);
    CHECK(ef_view_release(&second_view) == EF_OK);

    first.cursor = SIZE_MAX;
    CHECK(ef_parse(&first, (const unsigned char *)"a", 1, &first_view) ==
          EF_FULL);
    return 0;
}

static int test_attempts(void)
{
    unsigned calls = 0;
    unsigned used = 99;
    CHECK(ef_attempt(callback, &calls, 1, 2, &used) == EF_INCOMPLETE &&
          used == 2 && calls == 2);
    CHECK(ef_attempt(callback, &calls, 1, 1, &used) == EF_OK && used == 1 &&
          calls == 3);
    CHECK(ef_attempt(callback, &calls, 0, 1, &used) == EF_INVALID &&
          calls == 3);
    CHECK(ef_attempt(callback, &calls, 1, 0, &used) == EF_INCOMPLETE &&
          used == 0);
    CHECK(ef_attempt(never_ready, NULL, 1, 16, &used) == EF_INCOMPLETE &&
          used == 16);
    return 0;
}

enum sort_shape
{
    SHAPE_ASCENDING,
    SHAPE_DESCENDING,
    SHAPE_REPEATING,
    SHAPE_EXTREMES,
    SHAPE_SCRAMBLED,
    SHAPE_COUNT
};

/* Deterministic input value i of one of the input shapes. */
static int shape_value(enum sort_shape shape, size_t i)
{
    switch (shape)
    {
        case SHAPE_ASCENDING:
            return (int)i;
        case SHAPE_DESCENDING:
            return (int)(16 - i);
        case SHAPE_REPEATING:
            return (int)(i % 3);
        case SHAPE_EXTREMES:
            return (i & 1) ? INT_MIN : INT_MAX;
        case SHAPE_SCRAMBLED:
        default:
            return (int)((i * 19 + 7) % 13);
    }
}

static int test_sort_integers(void)
{
    for (size_t count = 0; count <= 16; ++count)
    {
        for (int shape = 0; shape < SHAPE_COUNT; ++shape)
        {
            int input[17];
            int expected[17];
            for (size_t i = 0; i < 17; ++i)
            {
                input[i] = shape_value((enum sort_shape)shape, i);
            }
            memcpy(expected, input, sizeof input);
            qsort(expected, count, sizeof *expected, compare_int);
            /* Elements past count must stay as they were. */
            CHECK(ef_sort(input, count) == EF_OK &&
                  memcmp(input, expected, sizeof input) == 0);
        }
    }
    int rejected[17] = {2, 1};
    CHECK(ef_sort(rejected, 17) == EF_INVALID && rejected[0] == 2 &&
          rejected[1] == 1);
    return 0;
}

static int test_sort_records(void)
{
    struct ef_record records[16];
    for (unsigned i = 0; i < 16; ++i)
    {
        records[i].key = (int)(i % 3);
        records[i].identity = i;
        for (unsigned j = 0; j < EF_RECORD_PAYLOAD_BYTES; ++j)
        {
            records[i].payload[j] = (unsigned char)i;
        }
    }
    CHECK(ef_sort_records(records, 16) == EF_OK);
    for (unsigned i = 0; i < 16; ++i)
    {
        /* The payload travels with its record. */
        CHECK(records[i].payload[0] == records[i].identity);
        if (i)
        {
            CHECK(records[i - 1].key < records[i].key ||
                  (records[i - 1].key == records[i].key &&
                   records[i - 1].identity < records[i].identity));
        }
    }
    return 0;
}

static int test_logger(void)
{
    struct ef_log log = {ATOMIC_FLAG_INIT, {0}, 0};
    CHECK(ef_log(&log, (const unsigned char *)"event", 5) == 1 &&
          log.length == 5);
    /* Hold the flag as a second owner would; the logger must not wait. */
    CHECK(!atomic_flag_test_and_set_explicit(&log.owned, memory_order_acquire));
    CHECK(ef_log(&log, (const unsigned char *)"nested", 6) == 0 &&
          log.length == 5 && memcmp(log.bytes, "event", 5) == 0);
    atomic_flag_clear_explicit(&log.owned, memory_order_release);
    CHECK(ef_log(&log, NULL, 33) == -1 && log.length == 5);
    return 0;
}

int main(void)
{
    if (test_cache() || test_pool() || test_parser() || test_attempts() ||
        test_sort_integers() || test_sort_records() || test_logger())
    {
        return 1;
    }
    puts(
        "EFFECTS_CORE_PASS cache pool parser bounded callback stable sort logger");
    return 0;
}
