#include "routine.h"
extern uint32_t cl_stack_top, cl_data_start, cl_data_end, cl_data_load,
    cl_bss_start, cl_bss_end;
volatile uint32_t dc_input, dc_output, dc_counter, dc_done;
unsigned char dc_bytes[4];

__attribute__((noinline)) void dc_checkpoint(void)
{
    __asm__ volatile("nop" ::: "memory");
}

static void failed(void)
{
    for (;;)
    {
        __asm__ volatile("wfi");
    }
}

void cl_reset(void)
{
    uint32_t *s = &cl_data_load;
    for (uint32_t *d = &cl_data_start; d < &cl_data_end; ++d)
    {
        *d = *s++;
    }
    for (uint32_t *d = &cl_bss_start; d < &cl_bss_end; ++d)
    {
        *d = 0;
    }
    dc_bytes[0] = 1;
    dc_bytes[1] = 2;
    dc_bytes[2] = 3;
    dc_bytes[3] = 4;
    dc_input = 5;
    dc_counter = 0;
    dc_checkpoint();
    dc_output = dc_checksum(dc_bytes, 4);
    dc_checkpoint();
    dc_bytes[1] = 7;
    dc_output = dc_checksum(dc_bytes, 4);
    dc_checkpoint();
    dc_counter = 7;
    dc_checkpoint();
    dc_counter = 7;
    dc_checkpoint();
    dc_output = dc_tail(9);
    dc_done = 1;
    dc_checkpoint();
    for (;;)
    {
        __asm__ volatile("wfi");
    }
}

__attribute__((section(".vectors"), used))
const uintptr_t dc_vectors[16] = {[0] = (uintptr_t)&cl_stack_top,
                                  [1] = (uintptr_t)cl_reset,
                                  [2] = (uintptr_t)failed,
                                  [3] = (uintptr_t)failed};
