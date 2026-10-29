#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>
#include <stdatomic.h>
static atomic_uint counter;
static alignas(uint32_t) unsigned char storage[4];
_Static_assert(sizeof(unsigned) == 4, "selected M4 unsigned width");

static void say(const char *s)
{
    register unsigned r0 __asm__("r0") = 4;
    register const char *r1 __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

_Noreturn static void finish(unsigned status)
{
    unsigned args[2] = {0x20026, status};
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
            say("LP_TARGET_CHECK " #x "\n"); \
            finish(1);                       \
        }                                    \
    } while (0)

static unsigned bounded(size_t n)
{
    if (n == 0 || n > 16)
        return 999;
    unsigned a[n], sum = 0;
    for (size_t i = 0; i < n; i++)
    {
        a[i] = (unsigned)i;
        sum += a[i];
    }
    return sum;
}

void lp_target(void)
{
    atomic_init(&counter, 0);
    TEST(atomic_fetch_add(&counter, 3) == 0 && atomic_load(&counter) == 3);
    TEST((uintptr_t)storage % alignof(uint32_t) == 0);
    int n = 0;
    TEST(_Generic(n++, int: 1, default: 0) == 1 && n == 0);
    TEST(bounded(0) == 999 && bounded(17) == 999 && bounded(16) == 120);
    say("LP_TARGET_PASS selected32 no-library helpers audited\n");
    finish(0);
}
