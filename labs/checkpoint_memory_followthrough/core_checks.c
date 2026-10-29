#include "checks.h"
#include "checkpoint_memory.h"
#include <stdint.h>
#define REQUIRE(x)           \
    do                       \
    {                        \
        if (!(x))            \
            return __LINE__; \
    } while (0)

unsigned p1_checks(void)
{
    _Alignas(8) unsigned char storage[40];
    struct cpm_arena arena;
    struct cpm_span span = {917, 918, 919};
    for (size_t i = 0; i < sizeof storage; i++)
    {
        storage[i] = 0xa5;
    }
    REQUIRE(cpm_arena_init(&arena, storage, 32, 8, 8) == CPM_OK);
    REQUIRE(cpm_reserve(&arena, 24, &span) == CPM_OK && span.offset == 8 &&
            arena.used == 32);
    REQUIRE(storage[32] == 0xa5 && storage[39] == 0xa5);
    REQUIRE(cpm_arena_init(&arena, storage, 31, 8, 8) == CPM_OK);
    span = (struct cpm_span){917, 918, 919};
    unsigned char saved[40];
    for (size_t i = 0; i < 40; i++)
    {
        saved[i] = storage[i];
    }
    REQUIRE(cpm_reserve(&arena, 24, &span) == CPM_FULL && arena.used == 0 &&
            span.offset == 917);
    for (size_t i = 0; i < 40; i++)
    {
        REQUIRE(saved[i] == storage[i]);
    }
    REQUIRE(cpm_arena_init(&arena, storage, 32, 8, 16) == CPM_INVALID &&
            arena.capacity == 31 && arena.used == 0);
    REQUIRE(cpm_reserve(&arena, SIZE_MAX, &span) == CPM_INVALID &&
            arena.used == 0);
    REQUIRE(cpm_arena_init(&arena, storage, 32, 8, 8) == CPM_OK &&
            cpm_reserve(&arena, 5, &span) == CPM_OK);
    REQUIRE(cpm_pin(&arena, span) == CPM_OK && cpm_reset(&arena) == CPM_BUSY);
    REQUIRE(cpm_unpin(&arena) == CPM_OK && cpm_reset(&arena) == CPM_OK);
    const unsigned char *view = storage;
    REQUIRE(cpm_view(&arena, span, &view) == CPM_INVALID && view == storage);
    struct cpm_pool pool;
    struct cpm_ring ring;
    int values[] = {11, 22, 33, 44};
    for (unsigned fail = 1; fail <= 4; fail++)
    {
        cpm_pool_init(&pool);
        cpm_ring_init(&ring);
        cpm_fail_at(&pool, fail);
        REQUIRE(cpm_atomic(&pool, &ring, values, 4) == CPM_FULL &&
                ring.count == 0 && pool.attempts == 0);
        for (unsigned i = 0; i < 4; i++)
        {
            REQUIRE(pool.state[i] == 0 && pool.generation[i] == 1);
        }
    }
    cpm_pool_init(&pool);
    struct cpm_handle old = {0, 1}, h;
    for (unsigned g = 1; g <= 3; g++)
    {
        REQUIRE(cpm_allocate(&pool, 11, &h) == CPM_OK && h.slot == 0 &&
                h.generation == g);
        REQUIRE(cpm_release(&pool, h) == CPM_OK);
    }
    const struct cpm_record *record = 0;
    REQUIRE(pool.state[0] == 2 &&
            cpm_lookup(&pool, old, &record) == CPM_INVALID && record == 0);
    REQUIRE(cpm_allocate(&pool, 22, &h) == CPM_OK && h.slot == 1);
    /* Independent FIFO model uses values and shifts, not slots/generations/ring indices.
       Eight actions cannot both retire a slot and fill the reduced pool; this
       finite bound is explicit, not an unbounded capacity assertion. */
    for (unsigned trace = 0; trace < 6561; trace++)
    {
        cpm_pool_init(&pool);
        cpm_ring_init(&ring);
        unsigned code = trace, n = 0;
        int model[4] = {0};
        for (unsigned step = 0; step < 8; step++)
        {
            unsigned action = code % 3;
            code /= 3;
            if (action < 2)
            {
                int value = action ? 22 : 11;
                size_t accepted = 917;
                enum cpm_status result =
                    cpm_prefix(&pool, &ring, &value, 1, &accepted);
                REQUIRE(result == (n == 4 ? CPM_FULL : CPM_OK) &&
                        accepted == (n == 4 ? 0u : 1u));
                if (n < 4)
                {
                    model[n++] = value;
                }
            }
            else
            {
                enum cpm_status result = cpm_pop(&ring, &h);
                REQUIRE(result == (n ? CPM_OK : CPM_FULL));
                if (n)
                {
                    REQUIRE(cpm_lookup(&pool, h, &record) == CPM_OK &&
                            record->value == model[0]);
                    REQUIRE(cpm_release(&pool, h) == CPM_OK);
                    for (unsigned i = 1; i < n; i++)
                    {
                        model[i - 1] = model[i];
                    }
                    --n;
                }
            }
            REQUIRE(ring.count == n);
            unsigned live = 0;
            for (unsigned i = 0; i < 4; i++)
            {
                live += pool.state[i] == 1;
            }
            REQUIRE(live == n);
        }
        while (n)
        {
            REQUIRE(cpm_pop(&ring, &h) == CPM_OK &&
                    cpm_lookup(&pool, h, &record) == CPM_OK &&
                    record->value == model[0]);
            REQUIRE(cpm_release(&pool, h) == CPM_OK);
            for (unsigned i = 1; i < n; i++)
            {
                model[i - 1] = model[i];
            }
            --n;
        }
    }
    return 0;
}
