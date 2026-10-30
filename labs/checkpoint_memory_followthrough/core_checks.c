#include "checks.h"
#include "checkpoint_memory.h"
#include <stdint.h>

/* A failed requirement returns the source line of the check (never 0). */
#define REQUIRE(x)           \
    do                       \
    {                        \
        if (!(x))            \
        {                    \
            return __LINE__; \
        }                    \
    } while (0)

/* Runs one check function; propagates its nonzero line number. */
#define RUN_CHECK(check)         \
    do                           \
    {                            \
        unsigned line = (check); \
        if (line)                \
        {                        \
            return line;         \
        }                        \
    } while (0)

enum
{
    STORAGE_BYTES = 40, /* backing array: 32 usable + 8 guard bytes */
    ARENA_BYTES = 32,
    ARENA_ALIGN = 8,
    GUARD_FILL = 0xa5, /* pattern that proves nothing wrote past 32 */
    UNSET = 917, /* sentinel for outputs that must stay untouched */
    POOL_SLOTS = 4,
    FIFO_DEPTH = 8, /* actions per trace */
    ACTION_KINDS = 3, /* offer 11, offer 22, take */
    TRACE_COUNT = 6561 /* ACTION_KINDS ** FIFO_DEPTH */
};

/* Slot lifecycle values stored in pool.state[]. */
enum
{
    SLOT_FREE = 0,
    SLOT_LIVE = 1,
    SLOT_RETIRED = 2
};

static unsigned check_arena_bounds(void)
{
    _Alignas(8) unsigned char storage[STORAGE_BYTES];
    unsigned char saved[STORAGE_BYTES];
    struct cpm_arena arena;
    struct cpm_span span = {UNSET, UNSET + 1, UNSET + 2};
    for (size_t i = 0; i < sizeof storage; i++)
    {
        storage[i] = GUARD_FILL;
    }
    /* Exact fit: 8 header bytes plus 24 payload bytes fill all 32. */
    REQUIRE(cpm_arena_init(&arena, storage, ARENA_BYTES, ARENA_ALIGN, 8) ==
            CPM_OK);
    REQUIRE(cpm_reserve(&arena, 24, &span) == CPM_OK && span.offset == 8 &&
            arena.used == 32);
    REQUIRE(storage[32] == GUARD_FILL && storage[39] == GUARD_FILL);
    /* One byte short: refused, arena and storage untouched. */
    REQUIRE(cpm_arena_init(&arena, storage, 31, ARENA_ALIGN, 8) == CPM_OK);
    span = (struct cpm_span){UNSET, UNSET + 1, UNSET + 2};
    for (size_t i = 0; i < sizeof storage; i++)
    {
        saved[i] = storage[i];
    }
    REQUIRE(cpm_reserve(&arena, 24, &span) == CPM_FULL && arena.used == 0 &&
            span.offset == UNSET);
    for (size_t i = 0; i < sizeof storage; i++)
    {
        REQUIRE(saved[i] == storage[i]);
    }
    /* Invalid parameters leave the previous arena configuration in place. */
    REQUIRE(cpm_arena_init(&arena, storage, ARENA_BYTES, ARENA_ALIGN, 16) ==
                CPM_INVALID &&
            arena.capacity == 31 && arena.used == 0);
    REQUIRE(cpm_reserve(&arena, SIZE_MAX, &span) == CPM_INVALID &&
            arena.used == 0);
    return 0;
}

/* A pinned span blocks reset; an unpinned one does not hand out raw bytes. */
static unsigned check_pin_and_view(void)
{
    _Alignas(8) unsigned char storage[STORAGE_BYTES];
    struct cpm_arena arena;
    struct cpm_span span;
    REQUIRE(cpm_arena_init(&arena, storage, ARENA_BYTES, ARENA_ALIGN, 8) ==
                CPM_OK &&
            cpm_reserve(&arena, 5, &span) == CPM_OK);
    REQUIRE(cpm_pin(&arena, span) == CPM_OK && cpm_reset(&arena) == CPM_BUSY);
    REQUIRE(cpm_unpin(&arena) == CPM_OK && cpm_reset(&arena) == CPM_OK);
    const unsigned char *view = storage;
    REQUIRE(cpm_view(&arena, span, &view) == CPM_INVALID && view == storage);
    return 0;
}

/* A failed acquisition at any of the four cuts rolls everything back. */
static unsigned check_atomic_cuts(void)
{
    struct cpm_pool pool;
    struct cpm_ring ring;
    int values[POOL_SLOTS] = {11, 22, 33, 44};
    for (unsigned fail = 1; fail <= POOL_SLOTS; fail++)
    {
        cpm_pool_init(&pool);
        cpm_ring_init(&ring);
        cpm_fail_at(&pool, fail);
        REQUIRE(cpm_atomic(&pool, &ring, values, POOL_SLOTS) == CPM_FULL &&
                ring.count == 0 && pool.attempts == 0);
        for (unsigned i = 0; i < POOL_SLOTS; i++)
        {
            REQUIRE(pool.state[i] == SLOT_FREE && pool.generation[i] == 1);
        }
    }
    return 0;
}

/* Slot 0 is reused for generations 1..3, then retired; old handles die. */
static unsigned check_generation_retirement(void)
{
    struct cpm_pool pool;
    cpm_pool_init(&pool);
    struct cpm_handle old = {0, 1};
    struct cpm_handle h;
    const struct cpm_record *record = 0;
    for (unsigned g = 1; g <= 3; g++)
    {
        REQUIRE(cpm_allocate(&pool, 11, &h) == CPM_OK && h.slot == 0 &&
                h.generation == g);
        REQUIRE(cpm_release(&pool, h) == CPM_OK);
    }
    REQUIRE(pool.state[0] == SLOT_RETIRED &&
            cpm_lookup(&pool, old, &record) == CPM_INVALID && record == 0);
    REQUIRE(cpm_allocate(&pool, 22, &h) == CPM_OK && h.slot == 1);
    return 0;
}

/*
 * Independent FIFO model: it holds only the offered values, shifted left on
 * each take, and knows nothing about slots, generations or ring indices.
 */
struct fifo_model
{
    int values[POOL_SLOTS];
    unsigned count;
};

static void model_push(struct fifo_model *m, int value)
{
    m->values[m->count++] = value;
}

static int model_front(const struct fifo_model *m)
{
    return m->values[0];
}

static void model_pop(struct fifo_model *m)
{
    for (unsigned i = 1; i < m->count; i++)
    {
        m->values[i - 1] = m->values[i];
    }
    --m->count;
}

/* `h` (just popped from the ring) must be the model's oldest value. */
static unsigned release_front(struct cpm_pool *pool, struct cpm_handle h,
                              struct fifo_model *model)
{
    const struct cpm_record *record = 0;
    REQUIRE(cpm_lookup(pool, h, &record) == CPM_OK &&
            record->value == model_front(model));
    REQUIRE(cpm_release(pool, h) == CPM_OK);
    model_pop(model);
    return 0;
}

static unsigned check_one_trace(unsigned trace)
{
    struct cpm_pool pool;
    struct cpm_ring ring;
    struct fifo_model model = {{0}, 0};
    cpm_pool_init(&pool);
    cpm_ring_init(&ring);
    unsigned code = trace;
    for (unsigned step = 0; step < FIFO_DEPTH; step++)
    {
        unsigned action = code % ACTION_KINDS;
        code /= ACTION_KINDS;
        if (action < 2)
        {
            int value = action ? 22 : 11;
            size_t accepted = UNSET;
            int full = model.count == POOL_SLOTS;
            enum cpm_status result =
                cpm_prefix(&pool, &ring, &value, 1, &accepted);
            REQUIRE(result == (full ? CPM_FULL : CPM_OK) &&
                    accepted == (full ? 0u : 1u));
            if (!full)
            {
                model_push(&model, value);
            }
        }
        else
        {
            struct cpm_handle h;
            enum cpm_status result = cpm_pop(&ring, &h);
            REQUIRE(result == (model.count ? CPM_OK : CPM_FULL));
            if (model.count)
            {
                RUN_CHECK(release_front(&pool, h, &model));
            }
        }
        REQUIRE(ring.count == model.count);
        unsigned live = 0;
        for (unsigned i = 0; i < POOL_SLOTS; i++)
        {
            live += pool.state[i] == SLOT_LIVE;
        }
        REQUIRE(live == model.count);
    }
    while (model.count)
    {
        struct cpm_handle h;
        REQUIRE(cpm_pop(&ring, &h) == CPM_OK);
        RUN_CHECK(release_front(&pool, h, &model));
    }
    return 0;
}

/*
 * All 3^8 = 6561 histories of eight offer/offer/take actions. Eight actions
 * cannot both retire a slot and fill the reduced pool, so this finite bound
 * is explicit and is not an unbounded capacity assertion.
 */
static unsigned check_fifo_traces(void)
{
    for (unsigned trace = 0; trace < TRACE_COUNT; trace++)
    {
        RUN_CHECK(check_one_trace(trace));
    }
    return 0;
}

unsigned p1_checks(void)
{
    RUN_CHECK(check_arena_bounds());
    RUN_CHECK(check_pin_and_view());
    RUN_CHECK(check_atomic_cuts());
    RUN_CHECK(check_generation_retirement());
    RUN_CHECK(check_fifo_traces());
    return 0;
}
