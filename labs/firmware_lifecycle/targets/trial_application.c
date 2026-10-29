#include <stdint.h>
#ifndef APP_CORE
#define APP_CORE 0
#endif
#if defined(TH_GUEST)
extern _Noreturn void th_guest_report(uint32_t, uint32_t);
#endif
#ifndef APP_GENERATION
#define APP_GENERATION 2
#endif
static const uint32_t input[8] = {
#if APP_CORE == 0
    1, 2, 3, 4, 5,
    6, 7, 8
#else
    8, 7, 6, 5, 4,
    3, 2, 1
#endif
};
_Noreturn void th_app_main(void)
{
    const volatile uint32_t *data = input;
    uint32_t answer = 2166136261u;
    for (unsigned i = 0; i < 8; ++i)
    {
        answer = (answer ^ data[i]) * 16777619u;
    }
#if defined(APP_BAD)
    answer ^= 1;
#endif
#if defined(TH_GUEST)
    th_guest_report(answer, APP_GENERATION);
#else
    typedef void (*f4_report_fn)(uint32_t, uint32_t);
    f4_report_fn report =
        (f4_report_fn)(uintptr_t)(APP_CORE ? 0x08100401u : 0x08120401u);
    report(answer, APP_GENERATION);
#endif
    for (;;)
    {
        __asm__ volatile("wfi");
    }
}
