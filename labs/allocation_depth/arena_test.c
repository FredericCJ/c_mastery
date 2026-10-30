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
    SENTINEL_BEGIN = 91, /* outputs that must survive a rejected call */
    SENTINEL_END = 92,
    EXHAUSTIVE_MAX_CAPACITY = 32,
    EXHAUSTIVE_MAX_BYTES = 33,
    EXHAUSTIVE_MAX_ALIGN = 16,
    DATA_BYTES = 64
};

/* Order in which cleanup callbacks ran, by the value each carries. */
static unsigned log_values[32];
static unsigned log_n;

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

/*
 * One arena object is carried through the phases in order: later phases
 * depend on the counters left by earlier ones (for example `peak`).
 */
struct fixture
{
    _Alignas(8) unsigned char data[DATA_BYTES];
    struct al_arena arena;
    struct al_arena other;
    struct al_handle handles[16];
    struct al_handle out;
};

static int local_call(struct al_arena *a)
{
    struct al_handle h;
    return al_arena_take(a, 1, 1, 0, 0, &h);
}

static int check_extent_examples(void)
{
    size_t at = SENTINEL_BEGIN;
    size_t end = SENTINEL_END;
    CHECK(al_extent(3, 5, 8, 24, &at, &end) == AL_OK && at == 8 && end == 13);
    CHECK(al_extent(SIZE_MAX - 2, 1, 8, SIZE_MAX, &at, &end) == AL_FULL &&
          at == 8 && end == 13);
    CHECK(al_extent(0, 0, 8, 24, &at, &end) == AL_BAD);
    CHECK(al_extent(0, 1, 3, 24, &at, &end) == AL_BAD);
    return 0;
}

/*
 * Independent model: round the cursor up by repeated increment, then compare
 * with al_extent for every small capacity, cursor, size and alignment.
 */
static int check_extent_exhaustive(unsigned *cases)
{
    for (size_t cap = 0; cap <= EXHAUSTIVE_MAX_CAPACITY; ++cap)
    {
        for (size_t cursor = 0; cursor <= cap; ++cursor)
        {
            for (size_t bytes = 1; bytes <= EXHAUSTIVE_MAX_BYTES; ++bytes)
            {
                for (size_t align = 1; align <= EXHAUSTIVE_MAX_ALIGN;
                     align *= 2)
                {
                    size_t want = cursor;
                    while (want % align)
                    {
                        ++want;
                    }
                    int fit = want <= cap && bytes <= cap - want;
                    size_t at = SENTINEL_BEGIN;
                    size_t end = SENTINEL_END;
                    int status =
                        al_extent(cursor, bytes, align, cap, &at, &end);
                    CHECK(status == (fit ? AL_OK : AL_FULL));
                    CHECK(fit ? (at == want && end == want + bytes)
                              : (at == SENTINEL_BEGIN && end == SENTINEL_END));
                    ++*cases;
                }
            }
        }
    }
    return 0;
}

static int check_take_view_reset(struct fixture *f)
{
    struct al_arena *a = &f->arena;
    struct al_handle *h = f->handles;
    unsigned char *p;
    size_t n;
    CHECK(al_arena_init(a, f->data, 24, 8) == AL_OK);
    CHECK(al_arena_take(a, 3, 1, 0, 0, &h[0]) == AL_OK);
    CHECK(al_arena_take(a, 5, 8, 0, 0, &h[1]) == AL_OK);
    struct al_arena saved = *a;
    CHECK(al_arena_take(a, 10, 4, 0, 0, &f->out) == AL_FULL &&
          memcmp(a, &saved, sizeof *a) == 0);
    CHECK(al_arena_take(a, 1, 1, 0, 0, &h[2]) == AL_OK && a->used == 14 &&
          a->payload == 9 && a->peak == 14);
    CHECK(al_arena_view(a, h[1], &p, &n) == AL_OK && p == f->data + 8 &&
          n == 5);
    CHECK(al_arena_reset(a) == AL_OK && a->peak == 14 && a->epoch_peak == 0);
    CHECK(al_arena_view(a, h[1], &p, &n) == AL_STALE);
    /* Alignment stronger than the arena's base alignment is refused. */
    CHECK(al_arena_take(a, 1, 16, 0, 0, &f->out) == AL_BAD);
    CHECK(al_arena_init(&f->other, f->data + 1, 23, 1) == AL_OK);
    CHECK(al_arena_take(&f->other, 1, 8, 0, 0, &f->out) == AL_BAD);
    return 0;
}

/* Cleanup order 2, 3, 1 across rollback, commit and an injected failure. */
static int check_marks_and_cleanup(struct fixture *f)
{
    struct al_arena *a = &f->arena;
    struct al_handle *h = f->handles;
    unsigned char *p;
    size_t n;
    struct al_mark m0;
    struct al_mark m1;
    struct al_mark foreign;
    struct al_cleanup c[3] = {{1, 0}, {2, 1}, {3, 0}};
    CHECK(al_arena_mark(a, &m0) == AL_OK);
    CHECK(al_arena_take(a, 2, 1, cleanup, &c[0], &h[0]) == AL_OK);
    CHECK(al_arena_mark(a, &m1) == AL_OK);
    CHECK(al_arena_take(a, 3, 1, cleanup, &c[1], &h[1]) == AL_OK);
    struct al_arena saved = *a;
    CHECK(al_arena_rollback(a, m0) == AL_STALE &&
          memcmp(a, &saved, sizeof *a) == 0);
    CHECK(al_arena_mark(&f->other, &foreign) == AL_OK);
    CHECK(al_arena_commit(a, foreign) == AL_STALE);
    CHECK(al_arena_rollback(a, m1) == AL_CLEANUP && log_n == 1 &&
          log_values[0] == 2);
    CHECK(al_arena_view(a, h[0], &p, &n) == AL_OK);
    CHECK(al_arena_view(a, h[1], &p, &n) == AL_STALE);
    CHECK(al_arena_mark(a, &m1) == AL_OK);
    CHECK(al_arena_take(a, 2, 1, cleanup, &c[2], &h[2]) == AL_OK);
    CHECK(al_arena_commit(a, m1) == AL_OK);
    CHECK(al_arena_rollback(a, m0) == AL_OK && log_n == 3 &&
          log_values[1] == 3 && log_values[2] == 1);
    return 0;
}

static int check_release_order(struct fixture *f)
{
    struct al_arena *a = &f->arena;
    struct al_handle *h = f->handles;
    CHECK(al_arena_take(a, 1, 1, 0, 0, &h[0]) == AL_OK);
    CHECK(al_arena_take(a, 1, 1, 0, 0, &h[1]) == AL_OK);
    CHECK(al_arena_take(a, 1, 1, 0, 0, &h[2]) == AL_OK);
    struct al_arena saved = *a;
    CHECK(al_arena_release(a, h[1]) == AL_BAD &&
          memcmp(a, &saved, sizeof *a) == 0);
    CHECK(al_arena_release(a, h[2]) == AL_OK);
    CHECK(al_arena_release(a, h[1]) == AL_OK);
    CHECK(al_arena_release(a, h[0]) == AL_OK);
    CHECK(al_arena_reset(a) == AL_OK);
    CHECK(local_call(a) == AL_OK && a->count == 1);
    CHECK(a->peak == 14 && a->epoch_peak == 1);
    CHECK(al_arena_reset(a) == AL_OK);
    return 0;
}

/* An exhausted serial counter retires identity creation; it never wraps. */
static int check_retirement(struct fixture *f)
{
    struct al_arena *a = &f->arena;
    struct al_mark mark;
    a->serial = UINT32_MAX;
    CHECK(al_arena_take(a, 1, 1, 0, 0, &f->out) == AL_RETIRED);
    CHECK(al_arena_mark(a, &mark) == AL_RETIRED);
    return 0;
}

int main(void)
{
    struct fixture f = {0};
    unsigned cases = 0;
    RUN_PHASE(check_extent_examples());
    RUN_PHASE(check_extent_exhaustive(&cases));
    RUN_PHASE(check_take_view_reset(&f));
    RUN_PHASE(check_marks_and_cleanup(&f));
    RUN_PHASE(check_release_order(&f));
    RUN_PHASE(check_retirement(&f));
    printf("PASS arena cases=%u cleanup-order=2,3,1 stale-and-retirement\n",
           cases);
    return 0;
}
