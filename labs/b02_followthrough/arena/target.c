#include "arena.h"

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

struct bf_resource
{
    struct bf_arena *a;
    struct bf_borrow b;
    unsigned calls, fail;
};

static int close_resource(void *v)
{
    struct bf_resource *r = v;
    unsigned char byte = 0;
    struct bf_borrow b;
    TEST(bf_arena_get(r->a, r->b, 0, &byte) == B2_OK && byte == 42 &&
         bf_arena_reserve(r->a, 1, 1, &b) == BF_ARENA_BUSY);
    ++r->calls;
    if (r->fail)
    {
        r->fail = 0;
        return 1;
    }
    return 0;
}

void bf_target(void)
{
    static struct bf_arena a;
    bf_arena_init(&a);
    struct b2_mark outer, inner;
    struct bf_borrow parent, child;
    unsigned char byte = 0;
    TEST(bf_arena_mark(&a, &outer) == B2_OK &&
         bf_arena_reserve(&a, 3, 1, &parent) == B2_OK &&
         bf_arena_set(&a, parent, 0, 9) == B2_OK);
    TEST(bf_arena_mark(&a, &inner) == B2_OK &&
         bf_arena_reserve(&a, 5, 8, &child) == B2_OK &&
         bf_arena_set(&a, child, 0, 42) == B2_OK && a.raw.cursor == 13);
    struct bf_resource r[2] = {{&a, child, 0, 0}, {&a, child, 0, 1}};
    TEST(bf_arena_register(&a, close_resource, &r[0]) == B2_OK &&
         bf_arena_register(&a, close_resource, &r[1]) == B2_OK);
    TEST(bf_arena_commit(&a, inner) == B2_OK && a.raw.cursor == 13 &&
         a.raw.depth == 1 && !r[0].calls && !r[1].calls);
    TEST(bf_arena_rollback(&a, outer) == BF_ARENA_CLEANUP &&
         a.raw.cursor == 13 && r[0].calls == 1 && r[1].calls == 1);
    TEST(bf_arena_get(&a, parent, 0, &byte) == B2_OK && byte == 9 &&
         bf_arena_rollback(&a, outer) == B2_OK && !a.raw.cursor &&
         r[0].calls == 1 && r[1].calls == 2);
    TEST(bf_arena_get(&a, child, 0, &byte) == B2_STALE &&
         bf_arena_mark(&a, &inner) == B2_OK &&
         bf_arena_reserve(&a, 1, 1, &parent) == B2_OK &&
         bf_arena_get(&a, child, 0, &byte) == B2_STALE);
    TEST(bf_arena_rollback(&a, inner) == B2_OK);
    say("BF_TARGET_PASS C17 M4 actual-B02 nested-commit retained-cleanup stale-borrow\n");
    finish(0);
}
