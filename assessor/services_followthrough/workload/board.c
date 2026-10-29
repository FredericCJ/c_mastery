#include "workload.h"
#define REG(a) (*(volatile uint32_t *)(uintptr_t)(a))
#define SH ((struct sf_work *)(uintptr_t)UINT32_C(0x30040000))

static uint32_t now(void)
{
    return REG(0xe0001004u);
}

static void barrier(void)
{
    __asm__ volatile("dsb sy\n\tisb sy" ::: "memory");
}

static _Noreturn void fail(void)
{
    __asm__ volatile("cpsid i");
    for (;;)
    {
        __asm__ volatile("wfi");
    }
}

void af_fault(void)
{
    fail();
}

void af_hsem(void)
{
    fail();
}

void af_systick(void)
{
    fail();
}

static void wait_phase(unsigned wanted)
{
    uint32_t start = now();
    while (atomic_load_explicit(&SH->phase, memory_order_acquire) != wanted)
    {
        if ((uint32_t)(now() - start) > 100000000u)
        {
            fail();
        }
    }
}

void af_entry(void)
{
    extern const unsigned af_vectors[];
    REG(0xe000ed08u) = (uint32_t)(uintptr_t)af_vectors;
    REG(0xe000edfcU) |= 1u << 24;
    REG(0xe0001000u) |= 1;
    REG(0xe0001004u) = 0;
    barrier();
#if SF_M7
    /* Exact cold entry requires BCM4 held and D-cache disabled. Shared SRAM3 is
     * noncacheable, shareable, XN on M7. Private DTCM is never peer/DMA payload. */
    if (REG(0xe000ed14u) & (1u << 16))
    {
        fail();
    }
    REG(0x580244dcu) |= 0xe0000000u;
    barrier();
    REG(0xe000ed94u) = 0;
    REG(0xe000ed98u) = 7;
    REG(0xe000ed9cu) = 0x30040000u;
    REG(0xe000eda0u) =
        (1u << 28) | (3u << 24) | (1u << 19) | (1u << 18) | (12u << 1) | 1u;
    REG(0xe000ed94u) = 5;
    barrier();
    sfw_init(SH);
    atomic_store_explicit(&SH->phase, 4, memory_order_release);
    REG(0x580244a0u) |= 8;
    barrier();
    wait_phase(5);
    atomic_store_explicit(&SH->phase, 0, memory_order_release);
#else
    wait_phase(4);
    atomic_store_explicit(&SH->phase, 5, memory_order_release);
#endif
#if (SF_M7 && SF_SWAP) || (!SF_M7 && !SF_SWAP)
    /* Acquisition/control owns offer/rejection. Two images exchange these roles;
     * clocks/boot/MPU authority stays M7, independent of logical service role. */
    while (SH->offered < 128)
    {
        wait_phase(0);
        if (sfw_offer(SH, SF_BURST))
        {
            fail();
        }
        atomic_store_explicit(&SH->phase, 1, memory_order_release);
    }
    wait_phase(0);
    atomic_store_explicit(&SH->phase, 2, memory_order_release);
    wait_phase(3);
#else
    for (;;)
    {
        unsigned phase;
        uint32_t start = now();
        do
        {
            phase = atomic_load_explicit(&SH->phase, memory_order_acquire);
            if ((uint32_t)(now() - start) > 100000000u)
            {
                fail();
            }
        } while (phase != 1 && phase != 2);
        if (phase == 2)
        {
            break;
        }
        if (sfw_work(SH))
        {
            fail();
        }
        atomic_store_explicit(&SH->phase, 0, memory_order_release);
    }
    while (af_queued(&SH->ring) > 2)
    {
        if (sfw_work(SH))
        {
            fail();
        }
    }
    if (sfw_quiescent(SH, 1, 1) || sfw_check(SH))
    {
        fail();
    }
    atomic_store_explicit(&SH->phase, 3, memory_order_release);
#endif
    if (sfw_check(SH))
    {
        fail();
    }
    for (;;)
    {
        __asm__ volatile("wfi");
    }
}
