/* Actual target receive/admit integration; deliberately no persistent commit.
 * Authenticated images remain staging artifacts for the next update phase. */
#include "selected_startup.h"
#include "recovery_transport.h"
static struct rt_frame frame;
volatile uint32_t rt_probe;
static const struct fb_policy policy = {
    .keys = {{0xd7, 0x5a, 0x98, 0x01, 0x82, 0xb1, 0x0a, 0xb7, 0xd5, 0x4b, 0xfe,
              0xd3, 0xc9, 0x64, 0x07, 0x3a, 0x0e, 0xe1, 0x72, 0xf3, 0xda, 0xa6,
              0x23, 0x25, 0xaf, 0x02, 0x1a, 0x68, 0xf7, 0x07, 0x51, 0x1a}},
    .present = 2,
    .floor = {1, 1},
    .generation_floor = 1};
static const struct bs_layouts layout = {
    .core = {{{0x08020200, 0x08022000, 0x20000000, 0x20020000},
              {0x08040200, 0x08042000, 0x20000000, 0x20020000}},
             {{0x08060200, 0x08062000, 0x30000000, 0x30020000},
              {0x08080200, 0x08082000, 0x30000000, 0x30020000}}}};

_Noreturn void st_main(void)
{
    struct fh_io io = st_target_io();
    if (!st_cold_h745(io) || rt_uart_init(io, 100000) != 1)
        for (;;)
            __asm__ volatile("wfi");
    rt_reset(&frame);
    unsigned budget = 10000000;
    uint32_t status;
    struct fb_image admitted;
    while (budget--)
    {
        int r = rt_uart_step(io, &frame, &status);
        if (r < 0)
        {
            rt_probe = 2;
            break;
        }
        if (r == 2)
        {
            rt_probe = rt_admit(&frame, &policy, &layout, &admitted) ? 1u : 3u;
            (void)rt_uart_tx(io, (unsigned char)rt_probe, 100000);
            break;
        }
    }
    if (!rt_probe)
        rt_probe = 4;
    for (;;)
        __asm__ volatile("wfi");
}
