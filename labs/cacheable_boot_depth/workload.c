#include "boot.h"
#ifndef CB_CORE
#define CB_CORE 0
#endif
#ifndef CB_GENERATION
#define CB_GENERATION 7u
#endif
/* Typed object in a fixed NOLOAD section, never included in BSS clearing. */
__attribute__((section(".peer"), aligned(32))) struct fw_peer cb_shared;
volatile uint32_t cb_status;

static _Noreturn void halt(uint32_t x)
{
    cb_status = x;
    __asm__ volatile("dsb sy" ::: "memory");
    for (;;)
        __asm__ volatile("wfi");
}

static enum fw_peer_result step_g(uint32_t g, enum fw_peer_op op)
{
    enum fw_peer_result r;
    unsigned budget = 1000000;
    do
    {
        r = fw_h745_step(g, op);
    } while (r == H7_BUSY && --budget);
    return r;
}

static enum fw_peer_result step(enum fw_peer_op op)
{
    return step_g(CB_GENERATION, op);
}

_Noreturn void cb_main(void)
{
    struct cb_io x = cb_target_io();
    if (!(CB_CORE ? cb_peer(x, CB_GENERATION) : cb_boot(x, CB_GENERATION)))
        halt(0xBAD00001u);
    if (step_g(CB_GENERATION - 1u, FW_BEGIN) != H7_STALE)
        halt(0xBAD00002u);
    if (step(FW_BEGIN) != H7_OK)
        halt(0xBAD00003u);
    if (fw_h745_step(CB_GENERATION, FW_HEALTHY) != H7_BUSY)
        halt(0xBAD00004u);
    if (step(FW_END) != H7_OK || step(FW_HEALTHY) != H7_OK)
        halt(0xBAD00005u);
    if (!CB_CORE && step(FW_CONFIRM) != H7_OK)
        halt(0xBAD00006u);
    halt(0x600D0000u | CB_CORE);
}
