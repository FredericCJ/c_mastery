#include "boot_program_mode.h"
#include <stdint.h>
#define W(a) (*(volatile uint32_t *)(uintptr_t)(a))
extern int fp_raw(uint32_t, uint32_t);
extern const unsigned char fp_store[], fp_abort[];

static _Noreturn void exit_guest(unsigned pass)
{
    const char *s =
        pass
            ? "FP_PASS: MPU refusal then trusted programming phase permits store\n"
            : "FP_BAD: missing privileged MPU phase rejected\n";
    register uint32_t r0 __asm__("r0") = 4;
    register const char *r1 __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
    uint32_t code[2] = {0x20026u, pass ? 0 : 1};
    register uint32_t r2 __asm__("r0") = 0x20;
    register uint32_t *r3 __asm__("r1") = code;
    __asm__ volatile("bkpt 0xab" : "+r"(r2) : "r"(r3) : "memory");
    for (;;)
    {
        __asm__ volatile("wfi");
    }
}

void fp_handler(uint32_t *frame, uint32_t ret)
{
    if (ret != 0xFFFFFFF9u || W(0xE000ED28u) != 0x82u ||
        W(0xE000ED34u) != 0x20010000u ||
        frame[6] != (uint32_t)(uintptr_t)fp_store)
    {
        exit_guest(0);
    }
    frame[6] = (uint32_t)(uintptr_t)fp_abort;
    W(0xE000ED28u) = 0x82u;
    __asm__ volatile("dsb sy\n\tisb sy" ::: "memory");
}

void fp_main(void)
{
    W(0xE000ED94u) = 0;
    W(0x20010000u) = 7;
    W(0xE000ED98u) = 0;
    W(0xE000ED9Cu) = 0x20010000u;
    W(0xE000EDA0u) = (6u << 24) | (1u << 28) | (4u << 1) | 1u;
    W(0xE000ED24u) |= 1u << 16;
    W(0xE000ED94u) = 5;
    __asm__ volatile("dsb sy\n\tisb sy" ::: "memory");
    if (fp_raw(0x20010000u, 9) || W(0x20010000u) != 7)
    {
        exit_guest(0);
    }
    fb_program_mode();
    unsigned permitted = fp_raw(0x20010000u, 11);
    exit_guest(permitted && W(0x20010000u) == 11);
}
