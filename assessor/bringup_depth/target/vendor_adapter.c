/* Original restricted adapter. It is NOT ST SystemInit or a libc
 * implementation. */
#include <stdint.h>
#ifndef BU_VENDOR_MPS2
#define BU_VENDOR_MPS2 0
#endif
#ifndef BU_VENDOR_SECOND
#define BU_VENDOR_SECOND 0
#endif
volatile uint32_t data = 0x12345678, bss;
__attribute__((section(".ccmram"))) volatile uint32_t second = 0x87654321;
extern uint32_t _sbss[], _ebss[], _sccmram[], _eccmram[], _siccmram[];
__attribute__((section(".retained"))) volatile uint32_t stage;

static void out(const char *s)
{
#if BU_VENDOR_MPS2
    register unsigned r0 __asm__("r0") = 4;
    register const char *r1 __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
#else
    (void)s;
#endif
}

static _Noreturn void end(unsigned status)
{
#if BU_VENDOR_MPS2
    uint32_t args[2] = {0x20026, status};
    register unsigned r0 __asm__("r0") = 0x20;
    register uint32_t *r1 __asm__("r1") = args;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
#else
    (void)status;
#endif
    for (;;)
    {
    }
}

void SystemInit(void)
{
    /* Pre-data/BSS hook: only constants, linker addresses and explicit stores. */
    stage = 1;
    for (uintptr_t p = (uintptr_t)_sbss; p < (uintptr_t)_ebss; p += 4)
    {
        *(volatile uint32_t *)p = 0xa5a5a5a5;
    }
    for (uintptr_t p = (uintptr_t)_sccmram; p < (uintptr_t)_eccmram; p += 4)
    {
        *(volatile uint32_t *)p = 0xa5a5a5a5;
    }
}

void __libc_init_array(void)
{
    /* Deliberately restricted original hook with no automatic runtime services.
     */
    if (stage != 1 || data != 0x12345678 || bss)
    {
        out("BU_VENDOR_BAD_INIT\n");
        end(1);
    }
    stage = 2;
#if BU_VENDOR_SECOND
    uintptr_t p = (uintptr_t)_sccmram, s = (uintptr_t)_siccmram;
    while (p < (uintptr_t)_eccmram)
    {
        *(volatile uint32_t *)p = *(const uint32_t *)s;
        p += 4;
        s += 4;
    }
#endif
}

int main(void)
{
    if (stage != 2)
    {
        out("BU_VENDOR_STAGE\n");
        end(1);
    }
    if (second != 0x87654321)
    {
        out("BU_VENDOR_SECOND_UNCOPIED\n");
        end(1);
    }
    stage = 3;
    out("BU_VENDOR_PASS data bss second stage\n");
    end(0);
}
