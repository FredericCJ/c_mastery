#include "boot_program_mode.h"
#include "f439_port.h"
#define WORD(a) (*(volatile uint32_t *)(uintptr_t)(a))
static struct f4_selection prior, selected;
static unsigned trial;
static struct f4_writer writer;
static struct rt_frame frame;

static _Noreturn void stop(void)
{
    for (;;)
        __asm__ volatile("wfi");
}

static int read_slot(void *c, uint32_t a, uint32_t *v)
{
    (void)c;
    return eg_read_word(a, v);
}
static const struct fb_policy policy = {
    .keys = {{0xd7, 0x5a, 0x98, 0x01, 0x82, 0xb1, 0x0a, 0xb7, 0xd5, 0x4b, 0xfe,
              0xd3, 0xc9, 0x64, 0x07, 0x3a, 0x0e, 0xe1, 0x72, 0xf3, 0xda, 0xa6,
              0x23, 0x25, 0xaf, 0x02, 0x1a, 0x68, 0xf7, 0x07, 0x51, 0x1a}},
    .present = 2,
    .floor = {1, 1},
    .generation_floor = 1};
static const struct fb_layout layouts[2] = {
    {0x08080200, 0x08082000, 0x20000000, 0x20020000},
    {0x080A0200, 0x080A2000, 0x20000000, 0x20020000}};

static void finish(enum fh_result e)
{
    unsigned n = 80000000;
    while (e == FH_PENDING && n--)
        e = f4_poll(&writer);
    if (e != FH_DONE)
        stop();
}

_Noreturn void st_main(void)
{
    struct fm_io io = f4_target_io();
    if (!f4_cold(io.bus))
        stop();
    WORD(0xE000ED24u) |= 1u << 17;
    __asm__ volatile("dsb sy\n\tisb sy" ::: "memory");
    if (f4_select(read_slot, 0, &policy, layouts, &prior) != 1)
        stop();
    if (!f4_uart_init(io.bus))
        stop();
    unsigned n = 1000000;
    unsigned char b = 0;
    uint32_t status = 0;
    int r = 0;
    while (n-- && (r = f4_uart_rx(io.bus, &b, &status)) == 0)
    {
    }
    if (r < 0)
        stop();
    if (r == 1)
    {
        if (b != 'M')
            stop();
        rt_reset(&frame);
        n = 10000000;
        int z = 0;
        while (n--)
        {
            r = f4_uart_rx(io.bus, &b, &status);
            if (r < 0)
                stop();
            if (r)
            {
                z = rt_byte(&frame, b);
                if (z < 0 || z == 2)
                    break;
            }
        }
        if (z != 2 || frame.bytes[5])
            stop();
        finish(f4_install_begin(&writer, io, &prior, &frame, &policy, layouts));
        struct fb_image installed;
        if (!f4_install_result(&writer, read_slot, 0, &policy, layouts,
                               &installed) ||
            !f4_uart_tx(io.bus, 5, 100000))
            stop();
        writer.state = 0;
        finish(f4_trial_begin(&writer, io, &prior, &installed));
        if (!f4_trial_probe(read_slot, 0, &policy, layouts, &prior, &selected))
            stop();
        trial = 1;
    }
    else
    {
        selected.sequence = prior.sequence;
        selected.record = prior.record;
        selected.image.version = prior.image.version;
        selected.image.generation = prior.image.generation;
        selected.image.base = prior.image.base;
        selected.image.stack = prior.image.stack;
        selected.image.entry = prior.image.entry;
        selected.image.extent = prior.image.extent;
        selected.image.key = prior.image.key;
        selected.image.core = prior.image.core;
        for (unsigned i = 0; i < 32; ++i)
            selected.image.digest[i] = prior.image.digest[i];
    }
    WORD(0x4000440Cu) = 0;
    __asm__ volatile("dsb sy" ::: "memory");
    fb_handoff(selected.image.base, selected.image.stack, selected.image.entry);
}

_Noreturn void th_report_main(uint32_t result, uint32_t generation)
{
    if (!trial || generation != selected.image.generation ||
        result != 671377293u)
        stop(); /* Privileged bank2 service leaves application RO flash MPU before programming. */
    fb_program_mode();
    writer.state = 0;
    finish(f4_commit_begin(&writer, f4_target_io(), &prior, &selected,
                           generation, result));
    __asm__ volatile("dsb sy" ::: "memory");
    WORD(0xE000ED0Cu) = 0x05FA0004u;
    __asm__ volatile("dsb sy\n\tisb sy" ::: "memory");
    stop();
}
