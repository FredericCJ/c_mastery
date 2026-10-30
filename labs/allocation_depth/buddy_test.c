#include "allocation.h"
#include <stdio.h>
#define CHECK(x)                                                         \
    do                                                                   \
    {                                                                    \
        if (!(x))                                                        \
        {                                                                \
            fprintf(stderr, "ASSERT buddy line %d: %s\n", __LINE__, #x); \
            return 1;                                                    \
        }                                                                \
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
    REGION_BYTES = 64,
    LEAF_BYTES = 8,
    LEAVES = REGION_BYTES / LEAF_BYTES,
    TAKE_STEP_BOUND = 15, /* nodes visited by one take: the whole tree */
    RELEASE_STEP_BOUND = 4 /* one step plus at most 3 levels climbed */
};

static int check_rounding_and_stale(void)
{
    struct al_buddy b;
    struct al_buddy other;
    struct al_handle a;
    struct al_handle c;
    size_t off;
    size_t charged;
    CHECK(al_buddy_init(&b) == AL_OK);
    CHECK(al_buddy_init(&other) == AL_OK);
    /* 9 bytes round up to a 16-byte buddy; the next 8 go right after it. */
    CHECK(al_buddy_take(&b, 9, &a, &off, &charged) == AL_OK && off == 0 &&
          charged == 16);
    CHECK(al_buddy_take(&b, 8, &c, &off, &charged) == AL_OK && off == 16 &&
          charged == 8);
    CHECK(al_buddy_release(&b, a) == AL_OK && al_buddy_check(&b) == AL_OK);
    CHECK(al_buddy_release(&other, c) == AL_STALE);
    CHECK(al_buddy_release(&b, c) == AL_OK && !b.state[0]);
    return 0;
}

/*
 * Fill all eight leaves, then free them in every one of the 256 subset
 * orders (the bits of `mask` first, the rest after). The tree must always
 * validate, offsets are relative to the region, and work stays bounded.
 */
static int check_release_histories(unsigned *histories)
{
    struct al_buddy b;
    struct al_handle h[LEAVES];
    size_t off;
    size_t charged;
    for (unsigned mask = 0; mask < (1u << LEAVES); ++mask)
    {
        CHECK(al_buddy_init(&b) == AL_OK);
        for (unsigned i = 0; i < LEAVES; ++i)
        {
            CHECK(al_buddy_take(&b, LEAF_BYTES, &h[i], &off, &charged) ==
                      AL_OK &&
                  off == i * LEAF_BYTES && charged == LEAF_BYTES &&
                  b.steps <= TAKE_STEP_BOUND);
        }
        for (unsigned i = 0; i < LEAVES; ++i)
        {
            if (mask & (1u << i))
            {
                CHECK(al_buddy_release(&b, h[i]) == AL_OK);
                CHECK(al_buddy_check(&b) == AL_OK &&
                      b.steps <= RELEASE_STEP_BOUND);
            }
        }
        for (unsigned i = 0; i < LEAVES; ++i)
        {
            if (!(mask & (1u << i)))
            {
                CHECK(al_buddy_release(&b, h[i]) == AL_OK);
                CHECK(al_buddy_check(&b) == AL_OK);
            }
        }
        CHECK(!b.state[0]);
        ++*histories;
    }
    return 0;
}

/* Each request 1..64 is charged a power of two no smaller than the request. */
static int check_request_sizes(void)
{
    struct al_buddy b;
    struct al_handle a;
    size_t off;
    size_t charged;
    for (unsigned request = 1; request <= REGION_BYTES; ++request)
    {
        CHECK(al_buddy_init(&b) == AL_OK);
        CHECK(al_buddy_take(&b, request, &a, &off, &charged) == AL_OK &&
              charged >= request && charged <= REGION_BYTES &&
              (charged & (charged - 1)) == 0);
        CHECK(al_buddy_release(&b, a) == AL_OK);
    }
    return 0;
}

int main(void)
{
    unsigned histories = 0;
    RUN_PHASE(check_rounding_and_stale());
    RUN_PHASE(check_release_histories(&histories));
    RUN_PHASE(check_request_sizes());
    printf(
        "PASS buddy histories=%u relative-offset/coalescence/operation-bounds\n",
        histories);
    return 0;
}
