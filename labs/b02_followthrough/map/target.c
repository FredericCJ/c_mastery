#include "map.h"

static void say(const char *s)
{
    register unsigned r0 __asm__("r0") = 4;
    register const char *r1 __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

static void finish(unsigned status)
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

void bf_target(void)
{
    struct b2_map m;
    struct b2_entry scratch[7];
    int out = 77;
    b2_map_init(&m);
    TEST(b2_map_put(&m, 0, 1) == B2_OK);
    TEST(b2_map_put(&m, 7, 2) == B2_OK);
    TEST(b2_map_remove(&m, 0) == B2_OK);
    TEST(bf_map_rehash(&m, scratch, 7, 1) == B2_FAILED);
    TEST(m.count == 1 && m.slots[0].state == 2);
    TEST(bf_map_rehash(&m, scratch, 7, 0) == B2_OK);
    TEST(b2_map_get(&m, 7, &out) == B2_OK && out == 2);
    struct bf_borrow_map borrowed = {0};
    struct cd_byte_map copied = {0};
    unsigned char bytes[2] = {65, 0}, old[2] = {65, 0};
    TEST(bf_borrow_put(&borrowed, bytes, 2, 19) == B2_OK);
    TEST(cd_bytemap_put(&copied, bytes, 2, 19) == CD_OK);
    bytes[0] = 66;
    TEST(bf_borrow_get(&borrowed, bytes, 2, &out) == B2_ABSENT);
    TEST(cd_bytemap_get(&copied, old, 2, &out) == CD_OK && out == 19);
    say("BF_TARGET_PASS C17 M4 actual-B02-CD-copy scratch-transaction\n");
    finish(0);
}
