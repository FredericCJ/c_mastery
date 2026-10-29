/* Original supplied register adapter. Learners implement the policy functions. */
#include "h745_cache.h"
#ifndef FW_H745_M7
#define FW_H745_M7 1
#endif
#define WORD(address) (*(volatile uint32_t *)(uintptr_t)(address))

static void barrier(void)
{
    __asm__ volatile("dsb sy" ::: "memory");
}

static void emit(void *unused, enum fw_cache_op op, uint32_t address)
{
    (void)unused;
    if (op == FW_DSB)
    {
        barrier();
    }
    else if (op == FW_ISB)
    {
        __asm__ volatile("isb sy" ::: "memory");
    }
    else if (op == FW_CLEAN)
    {
        WORD(0xE000EF68u) = address;
    }
    else
    {
        WORD(0xE000EF5Cu) = address;
    }
}

int fw_h745_cache(uint32_t a, uint32_t n, enum fw_cache_op op)
{
#if FW_H745_M7
    return fw_cache_range(a, n, op, emit, 0);
#else
    (void)
        emit; /* This M4 has no D-cache; completion still precedes publication. */
    if ((op != FW_CLEAN && op != H7_INVALIDATE) ||
        (n &&
         ((a & 31u) || (n & 31u) || n > 4096u || a > UINT32_MAX - (n - 1u))))
    {
        return 0;
    }
    barrier();
    return 1;
#endif
}

enum fw_peer_result fw_h745_step(uint32_t generation, enum fw_peer_op op)
{
    uint32_t saved;
    const uint32_t id = FW_H745_M7 ? 3u : 1u;
    __asm__ volatile("mrs %0, primask\n\tcpsid i" : "=r"(saved)::"memory");
    /* One serialized local user, process ID zero, semaphore zero reserved. */
    if (WORD(0x58026480u) != (0x80000000u | (id << 8)))
    {
        __asm__ volatile("msr primask, %0" ::"r"(saved) : "memory");
        return H7_BUSY;
    }
    barrier();
    (void)fw_h745_cache(0x30040000u, 32u, H7_INVALIDATE);
    enum fw_peer_result result =
        fw_peer_step((struct fw_peer *)(uintptr_t)0x30040000u,
                     FW_H745_M7 ? 0u : 1u, generation, op);
    (void)fw_h745_cache(0x30040000u, 32u, FW_CLEAN);
    barrier();
    WORD(0x58026400u) = id << 8;
    barrier();
    __asm__ volatile("msr primask, %0" ::"r"(saved) : "memory");
    return result;
}
