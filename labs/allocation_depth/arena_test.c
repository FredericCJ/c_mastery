#include "allocation.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                         \
    do                                                                   \
    {                                                                    \
        if (!(x))                                                        \
        {                                                                \
            fprintf(stderr, "ASSERT arena line %d: %s\n", __LINE__, #x); \
            return 1;                                                    \
        }                                                                \
    } while (0)
static unsigned log_values[32], log_n;

struct al_cleanup
{
    unsigned value;
    int fail;
};

static int cleanup(void *v)
{
    struct al_cleanup *c = v;
    log_values[log_n++] = c->value;
    return c->fail;
}

static int local_call(struct al_arena *a)
{
    struct al_handle h;
    return al_arena_take(a, 1, 1, 0, 0, &h);
}

int main(void)
{
    _Alignas(8) unsigned char data[64];
    struct al_arena a, other;
    struct al_handle h[16], out = {0};
    size_t at = 91, end = 92, n;
    unsigned char *p;
    CHECK(al_extent(3, 5, 8, 24, &at, &end) == AL_OK && at == 8 && end == 13);
    CHECK(al_extent(SIZE_MAX - 2, 1, 8, SIZE_MAX, &at, &end) == AL_FULL &&
          at == 8 && end == 13);
    CHECK(al_extent(0, 0, 8, 24, &at, &end) == AL_BAD);
    CHECK(al_extent(0, 1, 3, 24, &at, &end) == AL_BAD);
    unsigned cases = 0;
    for (size_t cap = 0; cap <= 32; ++cap)
        for (size_t c = 0; c <= cap; ++c)
            for (size_t bytes = 1; bytes <= 33; ++bytes)
                for (size_t align = 1; align <= 16; align *= 2)
                {
                    size_t want = c;
                    while (want % align)
                        ++want;
                    int fit = want <= cap && bytes <= cap - want;
                    at = 91;
                    end = 92;
                    int status = al_extent(c, bytes, align, cap, &at, &end);
                    CHECK(status == (fit ? AL_OK : AL_FULL));
                    CHECK(fit ? (at == want && end == want + bytes)
                              : (at == 91 && end == 92));
                    ++cases;
                }
    CHECK(al_arena_init(&a, data, 24, 8) == AL_OK);
    CHECK(al_arena_take(&a, 3, 1, 0, 0, &h[0]) == AL_OK);
    CHECK(al_arena_take(&a, 5, 8, 0, 0, &h[1]) == AL_OK);
    struct al_arena saved = a;
    CHECK(al_arena_take(&a, 10, 4, 0, 0, &out) == AL_FULL &&
          memcmp(&a, &saved, sizeof a) == 0);
    CHECK(al_arena_take(&a, 1, 1, 0, 0, &h[2]) == AL_OK && a.used == 14 &&
          a.payload == 9 && a.peak == 14);
    CHECK(al_arena_view(&a, h[1], &p, &n) == AL_OK && p == data + 8 && n == 5);
    CHECK(al_arena_reset(&a) == AL_OK && a.peak == 14 && a.epoch_peak == 0);
    CHECK(al_arena_view(&a, h[1], &p, &n) == AL_STALE);
    CHECK(al_arena_take(&a, 1, 16, 0, 0, &out) == AL_BAD);
    CHECK(al_arena_init(&other, data + 1, 23, 1) == AL_OK);
    CHECK(al_arena_take(&other, 1, 8, 0, 0, &out) == AL_BAD);
    struct al_mark m0, m1, foreign;
    struct al_cleanup c[3] = {{1, 0}, {2, 1}, {3, 0}};
    CHECK(al_arena_mark(&a, &m0) == AL_OK);
    CHECK(al_arena_take(&a, 2, 1, cleanup, &c[0], &h[0]) == AL_OK);
    CHECK(al_arena_mark(&a, &m1) == AL_OK);
    CHECK(al_arena_take(&a, 3, 1, cleanup, &c[1], &h[1]) == AL_OK);
    saved = a;
    CHECK(al_arena_rollback(&a, m0) == AL_STALE &&
          memcmp(&a, &saved, sizeof a) == 0);
    CHECK(al_arena_mark(&other, &foreign) == AL_OK);
    CHECK(al_arena_commit(&a, foreign) == AL_STALE);
    CHECK(al_arena_rollback(&a, m1) == AL_CLEANUP && log_n == 1 &&
          log_values[0] == 2);
    CHECK(al_arena_view(&a, h[0], &p, &n) == AL_OK);
    CHECK(al_arena_view(&a, h[1], &p, &n) == AL_STALE);
    CHECK(al_arena_mark(&a, &m1) == AL_OK);
    CHECK(al_arena_take(&a, 2, 1, cleanup, &c[2], &h[2]) == AL_OK);
    CHECK(al_arena_commit(&a, m1) == AL_OK);
    CHECK(al_arena_rollback(&a, m0) == AL_OK && log_n == 3 &&
          log_values[1] == 3 && log_values[2] == 1);
    CHECK(al_arena_take(&a, 1, 1, 0, 0, &h[0]) == AL_OK);
    CHECK(al_arena_take(&a, 1, 1, 0, 0, &h[1]) == AL_OK);
    CHECK(al_arena_take(&a, 1, 1, 0, 0, &h[2]) == AL_OK);
    saved = a;
    CHECK(al_arena_release(&a, h[1]) == AL_BAD &&
          memcmp(&a, &saved, sizeof a) == 0);
    CHECK(al_arena_release(&a, h[2]) == AL_OK);
    CHECK(al_arena_release(&a, h[1]) == AL_OK);
    CHECK(al_arena_release(&a, h[0]) == AL_OK);
    CHECK(al_arena_reset(&a) == AL_OK);
    CHECK(local_call(&a) == AL_OK && a.count == 1);
    CHECK(a.peak == 14 && a.epoch_peak == 1);
    CHECK(al_arena_reset(&a) == AL_OK);
    a.serial = UINT32_MAX;
    CHECK(al_arena_take(&a, 1, 1, 0, 0, &out) == AL_RETIRED);
    CHECK(al_arena_mark(&a, &m0) == AL_RETIRED);
    printf("PASS arena cases=%u cleanup-order=2,3,1 stale-and-retirement\n",
           cases);
    return 0;
}
