#include "target.h"
#ifndef CA_RELOAD
#define CA_RELOAD 25000u
#endif
extern uint32_t cl_stack_top, cl_data_start, cl_data_end, cl_data_load,
    cl_bss_start, cl_bss_end;
volatile uint32_t ca_ticks, ca_ready, ca_result, ca_fault;

__attribute__((noinline)) void ca_checkpoint(void)
{
    __asm__ volatile("nop" ::: "memory");
}

static void timer(void)
{
    *(volatile uint32_t *)UINT32_C(0x4000000c) = 1;
    ++ca_ticks;
}

static void failure(void)
{
    ca_ready = 3;
    for (;;)
    {
        __asm__ volatile("nop");
    }
}

void cl_reset(void)
{
    uint32_t *src = &cl_data_load;
    for (uint32_t *d = &cl_data_start; d < &cl_data_end; ++d)
    {
        *d = *src++;
    }
    for (uint32_t *d = &cl_bss_start; d < &cl_bss_end; ++d)
    {
        *d = 0;
    }
    ca_result = 0;
    ca_fault = 0;
    ca_ready = ca_target_paths() && ca_target_init(CA_RELOAD) ? 1 : 2;
    ca_checkpoint();
    for (;;)
    {
        __asm__ volatile("wfi");
    }
}

__attribute__((section(".vectors"), used)) const uintptr_t vectors[25] = {
    [0] = (uintptr_t)&cl_stack_top, [1] = (uintptr_t)cl_reset,
    [2] = (uintptr_t)failure,       [3] = (uintptr_t)failure,
    [4] = (uintptr_t)failure,       [5] = (uintptr_t)failure,
    [6] = (uintptr_t)failure,       [24] = (uintptr_t)timer};
