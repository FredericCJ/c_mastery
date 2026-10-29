#include "fixed_point.h"

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

void bf_target(void)
{
    const int8_t a[] = {1, 1}, b[] = {64, 64};
    int8_t out = 77;
    struct bf_signal signal;
    TEST(b2_q7(a, b, 2, &out) == B2_OK && out == 1 &&
         bf_signal(a, b, 2, &signal) == B2_OK && signal.output == 1 &&
         signal.error_numerator == 0);
    TEST(bf_q7_even(a, b, 1, &out) == B2_OK && out == 0);
    static int16_t wide[65536];
    for (unsigned i = 0; i < 65536; ++i)
    {
        wide[i] = -32768;
    }
    int16_t result = 77;
    TEST(bf_q11(wide, wide, 65536, &result) == B2_OK && result == 32767);
    int16_t x[8], y[8];
    for (unsigned i = 0; i < 8; ++i)
    {
        x[i] = -32768;
        y[i] = i < 4 ? -32768 : 32767;
    }
    TEST(bf_q11(x, y, 8, &result) == B2_OK && result == 64);
    struct bf_small small;
    struct bf_cascade cascade;
    TEST(bf_mul16(3, 8, BF_AWAY, &small) == B2_OK && small.output == 2 &&
         bf_mul16(1, 8, BF_EVEN, &small) == B2_OK && small.output == 0);
    TEST(bf_mix16_32(16, 32, BF_AWAY, &small) == B2_OK && small.output == 32);
    TEST(bf_two16(3, 8, 127, BF_AWAY, &cascade) == B2_OK &&
         cascade.error_numerator == 1048 && cascade.bound_numerator == 1144 &&
         cascade.bounded);
    say("BF_TARGET_PASS C17 M4 actual-B02 ties-even units Q11-full65536-exact\n");
    finish(0);
}
