#include "region.h"

static void say(const char *s)
{
    register unsigned r0 __asm__("r0") = 4;
    register const char *r1 __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

_Noreturn static void finish(unsigned status)
{
    unsigned args[2];
    args[0] = 0x20026;
    args[1] = status;
    register unsigned r0 __asm__("r0") = 0x20;
    register void *r1 __asm__("r1") = args;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
    for (;;)
    {
    }
}

#define TEST(x)                              \
    do                                       \
    {                                        \
        if (!(x))                            \
        {                                    \
            say("BF_TARGET_CHECK " #x "\n"); \
            finish(1);                       \
        }                                    \
    } while (0)

struct bf_pool
{
    struct bf_chunk chunks[3];
    unsigned used;
};

static struct bf_chunk *get(void *v)
{
    struct bf_pool *p = v;
    return p->used < 3 ? &p->chunks[p->used++] : 0;
}

static void put(void *v, struct bf_chunk *c)
{
    struct bf_pool *p = v;
    TEST(p->used && c == &p->chunks[p->used - 1]);
    --p->used;
}

struct bf_close
{
    unsigned calls, fail;
};

static int close_one(void *v)
{
    struct bf_close *c = v;
    ++c->calls;
    if (c->fail)
    {
        c->fail = 0;
        return 1;
    }
    return 0;
}

void bf_target(void)
{
    static struct bf_cohort c;
    bf_cohort_init(&c);
    const unsigned char wire[] = {2, 1, 0, 5, 2, 0, 7};
    unsigned total = 77;
    TEST(bf_cohort_batch(&c, wire, sizeof wire, 0, 0, &total) == MB_OK &&
         total == 12 && _Alignof(struct bf_object) >= 32);
    TEST(bf_cohort_reset(&c) == MB_OK && c.used == 2 && c.trace[0] == 2 &&
         c.trace[1] == 1 && !c.objects[0].live);
    TEST(bf_cohort_batch(&c, wire, sizeof wire, 0, 2, &total) == MB_FULL &&
         !c.objects[0].live && !c.objects[1].live &&
         c.objects[1].opens == c.objects[1].closes);
    static _Alignas(8) unsigned char backing[64];
    struct bf_slice s;
    struct bf_slice_span span;
    TEST(bf_slice_init(&s, backing, 64, 3, 29) == MB_OK &&
         bf_slice_alloc(&s, 5, 8, &span) == MB_OK && span.offset == 8 &&
         s.cursor == 10);
    static struct bf_region_tree tree;
    bf_tree_init(&tree);
    struct mb_span parent, child;
    TEST(mb_region_alloc(&tree.parent, 1, 1, &parent) == MB_OK &&
         mb_region_alloc(&tree.child[1], 1, 1, &child) == MB_OK);
    tree.parent.bytes[0] = 9;
    tree.child[1].bytes[0] = 8;
    unsigned char out = 0;
    TEST(bf_tree_child_reset(&tree, 0) == MB_OK &&
         mb_region_read(&tree.child[1], child, 0, &out) == MB_OK && out == 8 &&
         bf_tree_reset(&tree) == MB_OK &&
         mb_region_read(&tree.parent, parent, 0, &out) == MB_STALE);
    static struct bf_chunk first;
    static struct bf_pool pool;
    struct bf_growth g;
    struct bf_growth_span grow;
    TEST(bf_growth_init(&g, &first, get, put, &pool) == MB_OK);
    for (unsigned i = 0; i < 4; ++i)
        TEST(bf_growth_alloc(&g, 20, 8, &grow) == MB_OK);
    TEST(g.payload == 80 && g.capacity_high == 128 &&
         bf_growth_alloc(&g, 20, 8, &grow) == MB_FULL &&
         bf_growth_reset(&g) == MB_OK && !pool.used);
    static struct bf_retry_region retry;
    bf_retry_init(&retry);
    struct bf_borrow b;
    struct bf_close a = {0, 0}, failed = {0, 1};
    TEST(bf_retry_reserve(&retry, 8, 8, &b) == B2_OK &&
         bf_retry_register(&retry, close_one, &a) == B2_OK &&
         bf_retry_register(&retry, close_one, &failed) == B2_OK);
    TEST(bf_retry_reset(&retry) == BF_ARENA_CLEANUP &&
         retry.arena.raw.cursor == 8 && bf_retry_reset(&retry) == B2_OK &&
         a.calls == 1 && failed.calls == 2 && !retry.arena.raw.cursor);
    say("BF_TARGET_PASS C17 M4 actual-MB typed-align32 slices children growth retry\n");
    finish(0);
}
