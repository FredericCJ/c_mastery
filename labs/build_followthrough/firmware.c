#include "performance.h"
extern unsigned char __bss_start__, __bss_end__;
volatile uint32_t bf_board_input_a[5], bf_board_input_b[5];
static uint32_t out[5];
volatile uint32_t bf_board_result;

void reset(void)
{
    for (unsigned char *p = &__bss_start__; p < &__bss_end__; ++p)
    {
        *p = 0;
    }
    const uint32_t initial_a[5] = {0, UINT32_MAX, 4, 6, 8},
                   initial_b[5] = {1, 1, 5, 4, 2};
    uint32_t a[5], b[5];
    for (unsigned i = 0; i < 5; ++i)
    {
        bf_board_input_a[i] = initial_a[i];
        bf_board_input_b[i] = initial_b[i];
    }
    for (unsigned i = 0; i < 5; ++i)
    {
        a[i] = bf_board_input_a[i];
        b[i] = bf_board_input_b[i];
    }
    bf_add_disjoint(a, b, out, 5);
    bf_board_result = (out[0] == 1 && out[1] == 0 && out[2] == 9 &&
                       out[3] == 10 && out[4] == 10)
                          ? 0x50415353u
                          : 0x4641494cu;
    for (;;)
    {
        __asm__ volatile("wfi");
    }
}
