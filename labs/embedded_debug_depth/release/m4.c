#include "target.h"
extern uint32_t dt_stack_top, dt_data_start, dt_data_end, dt_data_load,
    dt_bss_start, dt_bss_end;
extern void dt_boot(void);

void dt_output(const char *s)
{
    register uintptr_t op __asm__("r0") = 4;
    register const char *arg __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(op) : "r"(arg) : "memory");
}

void dt_hex(uint32_t n)
{
    char text[9];
    for (unsigned i = 0; i < 8; i++)
    {
        text[i] = "0123456789abcdef"[(n >> ((7 - i) * 4)) & 15];
    }
    text[8] = 0;
    dt_output(text);
}

static void done(unsigned n)
{
    uint32_t block[] = {0x20026, n};
    register uintptr_t op __asm__("r0") = 0x20;
    register void *arg __asm__("r1") = block;
    __asm__ volatile("bkpt 0xab" : "+r"(op) : "r"(arg) : "memory");
    for (;;)
    {
        __asm__ volatile("wfi");
    }
}

static void failed(void)
{
    dt_output("DT_TARGET_CHECK:unexpected-vector\n");
    done(1);
}

void dt_reset(void)
{
    uint32_t *s = &dt_data_load;
    for (uint32_t *d = &dt_data_start; d < &dt_data_end; d++)
    {
        *d = *s++;
    }
    for (uint32_t *d = &dt_bss_start; d < &dt_bss_end; d++)
    {
        *d = 0;
    }
    int r = dt_target();
    dt_output(
        r ? "DT_TARGET_CHECK:contract\n"
          : "DT_TARGET_PASS:actual-interrupt-publication-and-software-reset\n");
    done(r ? 1 : 0);
}

__attribute__((section(".vectors"), used)) const uintptr_t dt_vectors[18] = {
    [0] = (uintptr_t)&dt_stack_top, [1] = (uintptr_t)dt_boot,
    [2] = (uintptr_t)failed,        [3] = (uintptr_t)failed,
    [16] = (uintptr_t)dt_irq0,      [17] = (uintptr_t)dt_irq1};
