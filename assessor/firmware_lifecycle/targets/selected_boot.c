#include "selected_startup.h"
#ifndef ST_CORE
#define ST_CORE 0
#endif
#define WORD(a) (*(volatile uint32_t *)(uintptr_t)(a))

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
/* RFC8032 TEST1 PUBLIC teaching key. Private signing is never linked here.
 * Deployment requires its own authenticated provisioning; no real key claimed. */
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
#if ST_CORE == 0
    if (!st_cold_h745(io))
        stop();
#else
    /* CPU2 was released only after master cold validation; no clock writes. */
    if (WORD(0xE000ED94u) || (WORD(0xE000ED14u) & 0x30000u))
        stop();
#endif
    WORD(0xE000ED24u) |= 1u << 17;
    __asm__ volatile("dsb sy\n\tisb sy" ::: "memory");
    const uint32_t records[2] = {0x080C0000, 0x080E0000};
    struct bs_selection selected;
    if (bs_select(read_slot, 0, &policy, &layout, records, &selected) != 1)
        stop();
    uint32_t generation = selected.image[0].generation;
#if ST_CORE == 0
    if (!st_release(io, &selected))
        stop();
#else
    if (!st_peer_matches(io, &selected))
        stop();
#endif
    unsigned left = 1000000;
    enum fw_peer_result result;
    do
    {
        result = fw_h745_step(generation, FW_HEALTHY);
    } while (result == H7_BUSY && --left);
    if (result != H7_OK)
        stop();
#if ST_CORE == 0
    left = 1000000;
    do
    {
        result = fw_h745_step(generation, FW_CONFIRM);
    } while (result == H7_BUSY && --left);
    if (result != H7_OK)
    {
        (void)fw_h745_step(generation, FW_FAIL_PEER);
        stop();
    }
    WORD(0x30040030u) = generation;
    __asm__ volatile("dsb sy" ::: "memory");
#else
    left = 1000000;
    while (WORD(0x30040030u) != generation && --left)
    {
    }
    if (!left)
        stop();
    __asm__ volatile("dmb sy" ::: "memory");
#endif
    /* Boot self-check acknowledgment is not application health certification.
     * No new persistent journal is committed by this boot-only coordinator. */
    fb_handoff(selected.image[ST_CORE].base, selected.image[ST_CORE].stack,
               selected.image[ST_CORE].entry);
}
