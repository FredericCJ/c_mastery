#include "facilities.h"

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

void lp_target(void)
{
    uint64_t q = 77;
    TEST(lf_div(UINT64_MAX, 3, &q) == LF_OK && q == 6148914691236517205ULL);
    TEST(lf_div(7, 0, &q) == LF_INVALID && q == 6148914691236517205ULL);
    int32_t s = 77;
    TEST(lf_add(INT32_MAX, 1, &s) == LF_INVALID && s == 77);
    uint16_t mixed = 77;
    TEST(lf_mixed(INT32_MIN, 2147483648u, &mixed) == LF_OK && mixed == 0);
    say("LP_TARGET_PASS facilities manual64 no implicit helper\n");
    finish(0);
}
