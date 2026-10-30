/* Public gates for the foundation_depth exercises. `tests <case>` runs one case
 * and exits 0 on success, 1 on the first failed check (printing an FD_CHECK
 * line to stderr) and 2 for a usage error.
 *
 * With FD_ASSESSOR_TESTS defined, the semantic case also checks the exact
 * verdict key supplied by the assessor tree. */
#include "depth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(x)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "FD_CHECK:%s:%d:%s\n", __FILE__, __LINE__, #x); \
            return 1;                                                       \
        }                                                                   \
    } while (0)
#define STATUS(expression, expected) CHECK((expression) == (expected))

/* Runs a helper that itself returns nonzero after a failed CHECK. */
#define RUN_STEP(call)   \
    do                   \
    {                    \
        if ((call) != 0) \
        {                \
            return 1;    \
        }                \
    } while (0)

/* Output locations are preloaded with these values so that a call which
 * wrongly writes on failure is caught. */
#define UNTOUCHED_SIZE ((size_t)917)
#define UNTOUCHED_INT 99
#define GUARD_BYTE 0xa5u

/* Exhaustive extent grid (size_t units): offset, count and width run below
 * EXTENT_GRID_MAX and limit below EXTENT_LIMIT_MAX; both bounds are exclusive,
 * small to enumerate yet large enough for products on both sides of limit. */
#define EXTENT_GRID_MAX 18u
#define EXTENT_LIMIT_MAX 32u

/* Overlap windows are enumerated inside a WINDOW_BYTES-byte array. */
#define WINDOW_BYTES 12u
#define OCTET_VALUES 256u

static int semantic(void)
{
#ifdef FD_ASSESSOR_TESTS
#include "semantic_answers.inc"
#else
    /* Public gate checks submission completeness only. Correct classification,
     * rationale and authority are separate assessor obligations. */
    for (unsigned i = 0; i < FD_CASE_COUNT; i++)
    {
        CHECK(fd_verdict((enum fd_case)i) != FD_UNKNOWN);
    }
#endif
    CHECK(fd_verdict((enum fd_case) - 1) == FD_UNKNOWN);

    int value = 5;
    int previous = UNTOUCHED_INT;
    STATUS(fd_repair_step(&value, &previous), FD_OK);
    CHECK(value == 6 && previous == 5);

    value = INT_MAX;
    previous = UNTOUCHED_INT;
    STATUS(fd_repair_step(&value, &previous), FD_RANGE);
    CHECK(value == INT_MAX && previous == UNTOUCHED_INT);
    STATUS(fd_repair_step(&value, &value), FD_INVALID);
    CHECK(value == INT_MAX);

    value = INT_MIN;
    STATUS(fd_repair_step(&value, &previous), FD_OK);
    CHECK(value == INT_MIN + 1 && previous == INT_MIN);
    return 0;
}

/* Every small combination against the exact (unwrapped) arithmetic. */
static int extent_grid(void)
{
    for (size_t offset = 0; offset < EXTENT_GRID_MAX; offset++)
    {
        for (size_t count = 0; count < EXTENT_GRID_MAX; count++)
        {
            for (size_t width = 0; width < EXTENT_GRID_MAX; width++)
            {
                for (size_t limit = 0; limit < EXTENT_LIMIT_MAX; limit++)
                {
                    size_t out = UNTOUCHED_SIZE;
                    const size_t exact = offset + count * width;
                    const int fits = exact <= limit;
                    const enum fd_status status =
                        fd_extent(offset, count, width, limit, &out);
                    CHECK(status == (fits ? FD_OK : FD_RANGE));
                    CHECK(out == (fits ? exact : UNTOUCHED_SIZE));
                }
            }
        }
    }
    return 0;
}

/* Arguments near SIZE_MAX, where the naive sum or product would wrap. */
static int extent_wrap_boundaries(void)
{
    size_t out = UNTOUCHED_SIZE;
    STATUS(fd_extent(SIZE_MAX, 1, 1, SIZE_MAX, &out), FD_RANGE);
    CHECK(out == UNTOUCHED_SIZE);
    STATUS(fd_extent(0, SIZE_MAX, 2, SIZE_MAX, &out), FD_RANGE);
    CHECK(out == UNTOUCHED_SIZE);
    STATUS(fd_extent(SIZE_MAX, SIZE_MAX, 0, SIZE_MAX, &out), FD_OK);
    CHECK(out == SIZE_MAX);
    STATUS(fd_extent(0, 0, SIZE_MAX, 0, &out), FD_OK);
    CHECK(out == 0);
    STATUS(fd_extent(SIZE_MAX, 0, 0, SIZE_MAX - 1, &out), FD_RANGE);
    CHECK(out == 0);
    STATUS(fd_extent(SIZE_MAX - 1, 1, 1, SIZE_MAX, &out), FD_OK);
    CHECK(out == SIZE_MAX);
    STATUS(fd_extent(0, 0, 0, 0, 0), FD_INVALID);
    return 0;
}

static int from_signed_cases(void)
{
    size_t out = UNTOUCHED_SIZE;
    STATUS(fd_from_signed(-1, &out), FD_RANGE);
    CHECK(out == UNTOUCHED_SIZE);
    STATUS(fd_from_signed(INTMAX_MIN, &out), FD_RANGE);
    CHECK(out == UNTOUCHED_SIZE);
    STATUS(fd_from_signed(0, &out), FD_OK);
    CHECK(out == 0);
    /* Whether INTMAX_MAX fits size_t depends on the target. */
    if ((uintmax_t)INTMAX_MAX > SIZE_MAX)
    {
        out = UNTOUCHED_SIZE;
        STATUS(fd_from_signed(INTMAX_MAX, &out), FD_RANGE);
        CHECK(out == UNTOUCHED_SIZE);
    }
    else
    {
        STATUS(fd_from_signed(INTMAX_MAX, &out), FD_OK);
        CHECK(out == (size_t)INTMAX_MAX);
    }
    return 0;
}

static int extent(void)
{
    RUN_STEP(extent_grid());
    RUN_STEP(extent_wrap_boundaries());
    RUN_STEP(from_signed_cases());
    return 0;
}

/* Take, then view the remainder, at every split point of a seven-byte array. */
static int cursor_splits(const unsigned char *bytes, size_t length)
{
    for (size_t split = 0; split <= length; split++)
    {
        struct fd_cursor cursor = {bytes, length, 0};
        struct fd_cursor view = {0, 0, 0};
        const unsigned char *taken = 0;
        STATUS(fd_take(&cursor, split, &taken), FD_OK);
        CHECK(taken == bytes && cursor.position == split);
        STATUS(fd_subview(&cursor, 0, length - split, &view), FD_OK);
        CHECK(cursor.position == split && view.base == bytes + split &&
              view.position == 0);
        STATUS(fd_take(&view, length - split, &taken), FD_OK);
        CHECK(taken == bytes + split);
        STATUS(fd_take(&view, 0, &taken), FD_OK);
        CHECK(taken == bytes + length);
    }
    return 0;
}

/* Rejected requests must leave the cursor, the view and the pointer alone. */
static int cursor_rejections_and_nesting(const unsigned char *bytes,
                                         size_t length)
{
    struct fd_cursor cursor = {bytes, length, 1};
    const unsigned char *taken = bytes + 4;
    STATUS(fd_take(&cursor, SIZE_MAX, &taken), FD_RANGE);
    CHECK(cursor.position == 1 && taken == bytes + 4);

    struct fd_cursor view = {bytes, 3, 2};
    STATUS(fd_subview(&cursor, SIZE_MAX, 0, &view), FD_RANGE);
    CHECK(view.base == bytes && view.length == 3 && view.position == 2);

    /* A subview of a subview, written back over its own parent. */
    STATUS(fd_subview(&cursor, 1, 4, &view), FD_OK);
    STATUS(fd_subview(&view, 1, 2, &view), FD_OK);
    CHECK(view.base == bytes + 3 && view.length == 2 && view.position == 0);
    return 0;
}

static int cursor_empty_and_invalid(const unsigned char *bytes)
{
    struct fd_cursor cursor = {0, 0, 0};
    struct fd_cursor view = {bytes, 3, 2};
    const unsigned char *taken = bytes + 4;
    STATUS(fd_take(&cursor, 0, &taken), FD_OK);
    CHECK(!taken && cursor.position == 0);
    STATUS(fd_subview(&cursor, 0, 0, &view), FD_OK);
    CHECK(!view.base && !view.length);
    STATUS(fd_take(&cursor, 1, &taken), FD_RANGE);
    CHECK(!taken);

    cursor = (struct fd_cursor){0, 1, 0};
    STATUS(fd_take(&cursor, 0, &taken), FD_INVALID);
    cursor = (struct fd_cursor){bytes, 1, 2};
    STATUS(fd_take(&cursor, 0, &taken), FD_INVALID);
    return 0;
}

static int cursor(void)
{
    const unsigned char bytes[] = {2, 3, 5, 7, 11, 13, 17};
    RUN_STEP(cursor_splits(bytes, sizeof bytes));
    RUN_STEP(cursor_rejections_and_nesting(bytes, sizeof bytes));
    RUN_STEP(cursor_empty_and_invalid(bytes));
    return 0;
}

static int record_equal(struct fd_record a, struct fd_record b)
{
    return a.flags == b.flags && a.count == b.count && a.value == b.value;
}

/* Fixed vectors decoded, encoded and decoded again. The encode target sits
 * inside a larger buffer so that stray writes to either neighbour show up. */
static int codec_known_vectors(void)
{
    static const unsigned char known[][FD_WIRE_BYTES] = {
        {1, 0, 0, 0, 0, 0, 0, 0},
        {1, 3, 0x12, 0x34, 0xff, 0xff, 0xff, 0xff},
        {1, 1, 0xff, 0xff, 0x80, 0, 0, 0},
        {1, 2, 0, 1, 0x7f, 0xff, 0xff, 0xff}};
    static const struct fd_record expected[] = {
        {0, 0, 0}, {3, 0x1234, -1}, {1, 65535, INT32_MIN}, {2, 1, INT32_MAX}};
    for (size_t i = 0; i < sizeof expected / sizeof expected[0]; i++)
    {
        struct fd_record result = {0, 0, 0};
        unsigned char wire[FD_WIRE_BYTES + 2];
        memset(wire, GUARD_BYTE, sizeof wire);
        STATUS(fd_decode(known[i], FD_WIRE_BYTES, &result), FD_OK);
        CHECK(record_equal(result, expected[i]));
        STATUS(fd_encode(expected[i], wire + 1, FD_WIRE_BYTES), FD_OK);
        CHECK(!memcmp(wire + 1, known[i], FD_WIRE_BYTES) &&
              wire[0] == GUARD_BYTE && wire[sizeof wire - 1] == GUARD_BYTE);
        result = (struct fd_record){0, 0, 0};
        STATUS(fd_decode(wire + 1, FD_WIRE_BYTES, &result), FD_OK);
        CHECK(record_equal(result, expected[i]));
    }
    return 0;
}

static int codec_decode_rejections(void)
{
    const struct fd_record untouched = {3, 999, 42};

    /* Every length except the exact record size, on a buffer long enough that
     * a decoder reading past the stated length would still find bytes. */
    for (size_t length = 0; length <= FD_WIRE_BYTES + 1; length++)
    {
        if (length != FD_WIRE_BYTES)
        {
            unsigned char wire[FD_WIRE_BYTES + 1] = {1, 0};
            struct fd_record result = untouched;
            STATUS(fd_decode(wire, length, &result), FD_RANGE);
            CHECK(record_equal(result, untouched));
        }
    }

    /* Sweep each header field (version, flags) over every octet value. */
    for (unsigned field = 0; field < 2; field++)
    {
        for (unsigned octet = 0; octet < OCTET_VALUES; octet++)
        {
            unsigned char wire[FD_WIRE_BYTES] = {1, 0};
            struct fd_record result = untouched;
            wire[field] = (unsigned char)octet;
            const int valid =
                field == 0 ? octet == FD_WIRE_VERSION : octet <= FD_FLAGS_MASK;
            STATUS(fd_decode(wire, FD_WIRE_BYTES, &result),
                   valid ? FD_OK : FD_INVALID);
            if (!valid)
            {
                CHECK(record_equal(result, untouched));
            }
        }
    }
    return 0;
}

static int codec_encode_rejections(void)
{
    unsigned char wire[FD_WIRE_BYTES];
    memset(wire, GUARD_BYTE, sizeof wire);
    STATUS(fd_encode((struct fd_record){FD_FLAGS_MASK + 1, 0, 0}, wire,
                     FD_WIRE_BYTES),
           FD_INVALID);
    for (size_t i = 0; i < sizeof wire; i++)
    {
        CHECK(wire[i] == GUARD_BYTE);
    }
    STATUS(fd_encode((struct fd_record){0, 0, 0}, wire, FD_WIRE_BYTES - 1),
           FD_RANGE);
    for (size_t i = 0; i < sizeof wire; i++)
    {
        CHECK(wire[i] == GUARD_BYTE);
    }
    return 0;
}

static int codec(void)
{
    RUN_STEP(codec_known_vectors());
    RUN_STEP(codec_decode_rejections());
    RUN_STEP(codec_encode_rejections());
    return 0;
}

/* Counting allocator and callbacks. Counters let a test prove that storage was
 * freed exactly once and that a destructor ran only when it should. */
struct fd_life
{
    unsigned fail_alloc;
    unsigned fail_init;
    unsigned allocations;
    unsigned frees;
    unsigned initializations;
    unsigned destructions;
    unsigned live; /* allocations not yet freed */
    struct fd_slot *slot;
};

/* Marks the value the initializer stores, so the destructor can tell that it
 * was handed a fully constructed object. */
#define LIFE_VALUE 37
#define LIFE_SERIAL 123

static void *life_allocate(void *context, size_t size_bytes, size_t alignment)
{
    struct fd_life *life = context;
    life->allocations++;
    if (size_bytes != sizeof(struct fd_value) ||
        alignment != _Alignof(struct fd_value) || life->fail_alloc)
    {
        return 0;
    }
    void *storage = malloc(size_bytes);
    if (storage)
    {
        life->live++;
    }
    return storage;
}

static void life_deallocate(void *context, void *storage)
{
    struct fd_life *life = context;
    life->frees++;
    life->live--;
    free(storage);
}

/* Fails if the slot is not in the acquired state when the callback runs. */
static int life_initialize(void *context, struct fd_value *value)
{
    struct fd_life *life = context;
    life->initializations++;
    if (life->slot->state != FD_ACQUIRED)
    {
        return 1;
    }
    value->value = LIFE_VALUE;
    value->serial = LIFE_SERIAL;
    return life->fail_init ? 1 : 0;
}

static void life_destroy(void *context, struct fd_value *value)
{
    struct fd_life *life = context;
    if (value->value == LIFE_VALUE && value->serial == LIFE_SERIAL)
    {
        life->destructions++;
    }
}

enum life_mode
{
    LIFE_ALLOCATION_FAILS,
    LIFE_INITIALIZER_FAILS,
    LIFE_SUCCEEDS,
    LIFE_MODE_COUNT
};

/* One complete slot history under the given failure mode. */
static int lifecycle_run(enum life_mode mode)
{
    struct fd_slot slot = FD_SLOT_INITIALIZER;
    struct fd_life life = {0};
    life.fail_alloc = mode == LIFE_ALLOCATION_FAILS;
    life.fail_init = mode == LIFE_INITIALIZER_FAILS;
    life.slot = &slot;
    const struct fd_allocator allocator = {&life, life_allocate,
                                           life_deallocate};
    const struct fd_value sentinel = {UNTOUCHED_INT, UNTOUCHED_INT};
    const struct fd_value *published = &sentinel;

    STATUS(fd_slot_publish(&slot, &published), FD_STATE);
    CHECK(published == &sentinel);
    enum fd_status status = fd_slot_acquire(&slot, allocator);
    if (life.fail_alloc)
    {
        CHECK(status == FD_NOMEM && slot.state == FD_EMPTY && !life.live);
        fd_slot_reset(&slot);
        CHECK(!life.frees);
        return 0;
    }
    CHECK(status == FD_OK && slot.state == FD_ACQUIRED && life.live == 1);
    STATUS(fd_slot_publish(&slot, &published), FD_STATE);
    CHECK(published == &sentinel);
    STATUS(fd_slot_acquire(&slot, allocator), FD_STATE);
    CHECK(life.allocations == 1);

    status = fd_slot_initialize(&slot, life_initialize, life_destroy, &life);
    if (life.fail_init)
    {
        CHECK(status == FD_INIT_FAILED && slot.state == FD_EMPTY &&
              !life.live && life.frees == 1 && !life.destructions);
    }
    else
    {
        CHECK(status == FD_OK && slot.state == FD_INITIALIZED);
        STATUS(fd_slot_publish(&slot, &published), FD_OK);
        CHECK(slot.state == FD_PUBLISHED && published->value == LIFE_VALUE &&
              published->serial == LIFE_SERIAL);
        STATUS(fd_slot_initialize(&slot, life_initialize, life_destroy, &life),
               FD_STATE);
        fd_slot_reset(&slot);
        CHECK(slot.state == FD_EMPTY && life.destructions == 1 &&
              life.frees == 1 && !life.live);
    }
    /* A second reset of an empty slot must not free or destroy again. */
    fd_slot_reset(&slot);
    CHECK(life.frees == 1 && life.initializations == 1);
    return 0;
}

/* Reset of an acquired (never initialized) slot frees without destroying. */
static int lifecycle_reset_acquired(void)
{
    struct fd_slot slot = FD_SLOT_INITIALIZER;
    struct fd_life life = {0};
    life.slot = &slot;
    STATUS(fd_slot_acquire(&slot, (struct fd_allocator){&life, life_allocate,
                                                        life_deallocate}),
           FD_OK);
    fd_slot_reset(&slot);
    CHECK(life.frees == 1 && !life.initializations && !life.destructions);
    return 0;
}

static int lifecycle(void)
{
    for (unsigned mode = 0; mode < LIFE_MODE_COUNT; mode++)
    {
        RUN_STEP(lifecycle_run((enum life_mode)mode));
    }
    RUN_STEP(lifecycle_reset_acquired());
    return 0;
}

/* Every (destination, source, count) window in a WINDOW_BYTES array against a
 * snapshot-copy model. Out-of-range windows must leave the array untouched. */
static int overlap_windows(void)
{
    for (size_t destination = 0; destination <= WINDOW_BYTES; destination++)
    {
        for (size_t source = 0; source <= WINDOW_BYTES; source++)
        {
            for (size_t count = 0; count <= WINDOW_BYTES; count++)
            {
                unsigned char array[WINDOW_BYTES];
                unsigned char snapshot[WINDOW_BYTES];
                unsigned char expected[WINDOW_BYTES];
                for (size_t i = 0; i < WINDOW_BYTES; i++)
                {
                    /* 17 is coprime to 256, so the bytes are distinct and a
                     * misplaced copy is visible. */
                    array[i] = (unsigned char)(i * 17 + 3);
                    snapshot[i] = array[i];
                    expected[i] = array[i];
                }
                const int valid = count <= WINDOW_BYTES - destination &&
                                  count <= WINDOW_BYTES - source;
                if (valid)
                {
                    for (size_t i = 0; i < count; i++)
                    {
                        expected[destination + i] = snapshot[source + i];
                    }
                }
                STATUS(fd_move(array, WINDOW_BYTES, destination, source, count),
                       valid ? FD_OK : FD_RANGE);
                CHECK(!memcmp(array, expected, WINDOW_BYTES));
            }
        }
    }
    STATUS(fd_move(0, 0, 0, 0, 0), FD_OK);
    STATUS(fd_move(0, 1, 0, 0, 0), FD_INVALID);
    unsigned char array[WINDOW_BYTES] = {0};
    STATUS(fd_move(array, WINDOW_BYTES, SIZE_MAX, 0, 0), FD_RANGE);
    return 0;
}

/* Every mask over every octet value, with guard bytes on both sides. */
static int overlap_transform(void)
{
    for (unsigned mask = 0; mask < OCTET_VALUES; mask++)
    {
        unsigned char source[OCTET_VALUES];
        unsigned char destination[OCTET_VALUES + 2];
        for (unsigned i = 0; i < OCTET_VALUES; i++)
        {
            source[i] = (unsigned char)i;
        }
        destination[0] = 77;
        destination[OCTET_VALUES + 1] = 88;
        fd_transform(destination + 1, source, OCTET_VALUES,
                     (unsigned char)mask);
        CHECK(destination[0] == 77 && destination[OCTET_VALUES + 1] == 88);
        for (unsigned i = 0; i < OCTET_VALUES; i++)
        {
            CHECK(destination[i + 1] == (unsigned char)(i ^ mask));
        }
    }
    fd_transform(0, 0, 0, 0);
    return 0;
}

static int overlap(void)
{
    RUN_STEP(overlap_windows());
    RUN_STEP(overlap_transform());
    return 0;
}

static int handle_equal(struct fd_handle a, struct fd_handle b)
{
    return a.owner == b.owner && a.index == b.index &&
           a.generation == b.generation;
}

#define HANDLE_HISTORY (FD_POOL_CAPACITY * FD_GENERATION_MAX)

/* Acquire and release every slot through every generation. Each handle ever
 * issued must stay stale, and the pool must end permanently full. Leaves the
 * last handle issued in *last. */
static int handles_exhaust(struct fd_pool *pool, const struct fd_pool *other,
                           struct fd_handle *last)
{
    struct fd_handle history[HANDLE_HISTORY];
    size_t issued = 0;
    struct fd_value out = {UNTOUCHED_INT, UNTOUCHED_INT};
    for (size_t slot = 0; slot < FD_POOL_CAPACITY; slot++)
    {
        for (unsigned generation = 1; generation <= FD_GENERATION_MAX;
             generation++)
        {
            const struct fd_value stored = {(int32_t)issued,
                                            (uint32_t)issued + 1};
            STATUS(fd_pool_acquire(pool, stored, last), FD_OK);
            CHECK(last->index == slot && last->generation == generation &&
                  last->owner == pool);
            STATUS(fd_pool_get(pool, *last, &out), FD_OK);
            CHECK(out.value == (int32_t)issued && out.serial == issued + 1);
            for (size_t i = 0; i < issued; i++)
            {
                out = (struct fd_value){UNTOUCHED_INT, UNTOUCHED_INT};
                STATUS(fd_pool_get(pool, history[i], &out), FD_STALE);
                CHECK(out.value == UNTOUCHED_INT &&
                      out.serial == UNTOUCHED_INT);
            }
            STATUS(fd_pool_get(other, *last, &out), FD_STALE);
            history[issued++] = *last;
            STATUS(fd_pool_release(pool, *last), FD_OK);
            STATUS(fd_pool_release(pool, *last), FD_STALE);
        }
    }
    return 0;
}

/* Misuse of an exhausted pool, plus an independent pool filled and drained. */
static int handles_exhausted_and_misused(struct fd_pool *pool,
                                         struct fd_pool *other,
                                         struct fd_handle last)
{
    struct fd_handle handle = last;
    STATUS(fd_pool_acquire(pool, (struct fd_value){0, 0}, &handle), FD_FULL);
    CHECK(handle_equal(handle, last));
    for (size_t i = 0; i < FD_POOL_CAPACITY; i++)
    {
        CHECK(pool->entries[i].retired && !pool->entries[i].live);
    }

    struct fd_value out = {UNTOUCHED_INT, UNTOUCHED_INT};
    STATUS(fd_pool_get(pool, (struct fd_handle){pool, SIZE_MAX, 1}, &out),
           FD_STALE);
    STATUS(fd_pool_get(0, handle, &out), FD_INVALID);
    CHECK(out.value == UNTOUCHED_INT && out.serial == UNTOUCHED_INT);
    STATUS(fd_pool_release(0, handle), FD_INVALID);
    STATUS(fd_pool_get(pool, handle, 0), FD_INVALID);

    struct fd_handle active[FD_POOL_CAPACITY];
    for (size_t i = 0; i < FD_POOL_CAPACITY; i++)
    {
        STATUS(fd_pool_acquire(other, (struct fd_value){0, 0}, &active[i]),
               FD_OK);
    }
    STATUS(fd_pool_acquire(other, (struct fd_value){0, 0}, &handle), FD_FULL);
    for (size_t i = 0; i < FD_POOL_CAPACITY; i++)
    {
        STATUS(fd_pool_release(other, active[i]), FD_OK);
    }
    return 0;
}

static int handles(void)
{
    struct fd_pool pool = {0};
    struct fd_pool other = {0};
    fd_pool_init(&pool);
    fd_pool_init(&other);
    struct fd_handle last = {&pool, 77, 77};
    RUN_STEP(handles_exhaust(&pool, &other, &last));
    RUN_STEP(handles_exhausted_and_misused(&pool, &other, last));
    return 0;
}

static int defense(void)
{
    const unsigned char valid[] = {1, 0, 0, 2, 0, 0, 0, 3};
    int32_t counter = 10;
    STATUS(fd_apply(valid, sizeof valid, 0, &counter), FD_DENIED);
    CHECK(counter == 10);
    STATUS(fd_apply(valid, sizeof valid, 1, &counter), FD_OK);
    CHECK(counter == 16);

    /* Every wrong length, with an authorized caller. */
    for (size_t length = 0; length <= FD_WIRE_BYTES + 1; length++)
    {
        if (length != FD_WIRE_BYTES)
        {
            const unsigned char supplied[FD_WIRE_BYTES + 1] = {1, 0, 0, 2, 0,
                                                               0, 0, 3, 0};
            counter = 10;
            STATUS(fd_apply(supplied, length, 1, &counter), FD_RANGE);
            CHECK(counter == 10);
        }
    }

    unsigned char invalid[FD_WIRE_BYTES] = {2, 0, 0, 2, 0, 0, 0, 3};
    counter = 10;
    STATUS(fd_apply(invalid, sizeof invalid, 1, &counter), FD_INVALID);
    CHECK(counter == 10);
    invalid[0] = 1;
    invalid[1] = 0x80;
    STATUS(fd_apply(invalid, sizeof invalid, 1, &counter), FD_INVALID);
    CHECK(counter == 10);

    /* 10 + 2 * INT32_MAX leaves int32_t. */
    const unsigned char overflow[] = {1, 0, 0, 2, 0x7f, 0xff, 0xff, 0xff};
    STATUS(fd_apply(overflow, sizeof overflow, 1, &counter), FD_RANGE);
    CHECK(counter == 10);

    /* A zero change still needs authority. */
    const unsigned char zero[] = {1, 3, 0, 0, 0x80, 0, 0, 0};
    STATUS(fd_apply(zero, sizeof zero, 0, &counter), FD_DENIED);
    CHECK(counter == 10);
    STATUS(fd_apply(zero, sizeof zero, 1, &counter), FD_OK);
    CHECK(counter == 10);
    return 0;
}

int main(int argc, char **argv)
{
    const struct
    {
        const char *name;
        int (*test)(void);
    } cases[] = {{"semantic", semantic},   {"extent", extent},
                 {"cursor", cursor},       {"codec", codec},
                 {"lifecycle", lifecycle}, {"overlap", overlap},
                 {"handles", handles},     {"defense", defense}};

    if (argc != 2)
    {
        return 2;
    }
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; i++)
    {
        if (!strcmp(argv[1], cases[i].name))
        {
            return cases[i].test();
        }
    }
    return 2;
}
