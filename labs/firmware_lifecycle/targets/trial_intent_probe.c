/* Original inactive-pair-to-durable-attempt bank2 probe. No launch or commit. */
#include "selected_startup.h"
#include "trial_intent.h"
static struct ft_writer writer;

static int slot_read(void *c, uint32_t a, uint32_t *out)
{
    (void)c;
    return eg_read_word(a, out);
}

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
    struct fh_io bus = st_target_io();
    if (!st_cold_h745(bus))
    {
        for (;;)
        {
            __asm__ volatile("wfi");
        }
    }
    *(volatile uint32_t *)(uintptr_t)0xE000ED24u |= 1u << 17;
    __asm__ volatile("dsb sy\n\tisb sy" ::: "memory");
    const uint32_t records[2] = {0x080C0000, 0x080E0000};
    struct bs_selection old, trial;
    struct fb_image installed[2];
    if (bs_select(slot_read, 0, &policy, &layout, records, &old) != 1)
    {
        for (;;)
        {
            __asm__ volatile("wfi");
        }
    }
    for (unsigned c = 0; c < 2; ++c)
    {
        unsigned s = old.image[c].base == layout.core[c][0].base ? 1 : 0;
        if (!bs_read_image(slot_read, 0, &policy, layout.core[c][s], c,
                           &installed[c]))
        {
            for (;;)
            {
                __asm__ volatile("wfi");
            }
        }
    }
    enum fh_result z = ft_begin(&writer, fm_h745_io(), &old, installed);
    unsigned budget = 40000000;
    while (z == FH_PENDING && budget--)
    {
        z = ft_poll(&writer);
    }
    rt_probe =
        z == FH_DONE && ft_probe(slot_read, 0, &policy, &layout, &old, &trial)
            ? 7u
            : 8u;
    for (;;)
    {
        __asm__ volatile("wfi");
    }
}
