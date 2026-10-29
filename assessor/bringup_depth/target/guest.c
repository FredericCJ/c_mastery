#include "bringup.h"
#include <stdint.h>
#ifndef BU_FP
#define BU_FP 0
#endif
#ifndef BU_GUARD_FAULT
#define BU_GUARD_FAULT 0
#endif
#ifndef BU_SOFTWARE_RESET
#define BU_SOFTWARE_RESET 0
#endif
#ifndef BU_QUIET
#define BU_QUIET 0
#endif
#ifndef BU_F439
#define BU_F439 0
#endif
#ifndef BU_RAM_VECTORS
#define BU_RAM_VECTORS 0
#endif
#ifndef BU_WATCHDOG
#define BU_WATCHDOG 0
#endif
#ifndef BU_MASK_GUARD
#define BU_MASK_GUARD 0
#endif
#define REG(a) (*(volatile uint32_t *)(uintptr_t)(a))
volatile uint32_t bu_data = 0x12345678;
__attribute__((section(".bank_data"))) volatile uint32_t bu_bank = 0x87654321;
volatile uint32_t bu_bss;
__attribute__((section(".bank_bss"))) volatile uint32_t bu_bank_bss;
__attribute__((section(".retained"))) volatile uint32_t bu_retained;
__attribute__((section(".retained"))) volatile uint32_t bu_reset_flags;
__attribute__((section(".stage"))) volatile uint32_t bu_stage;
__attribute__((section(".dma"), used)) volatile unsigned char bu_dma_buffer[16];
#if BU_RAM_VECTORS
__attribute__((aligned(512))) static uint32_t ram_vectors[107];
#endif
static volatile unsigned ticks, svc_count;
static volatile unsigned char scratch_sink;
extern unsigned char _stack_low[], _stack_top[];
extern const uint32_t bu_vectors_target[];

static void out(const char *s)
{
#if BU_F439
    (void)s; /* Physical channel is retained stage; no UART or debugger
                dependency. */
#else
    if (!BU_QUIET)
    {
        register unsigned r0 __asm__("r0") = 4;
        register const char *r1 __asm__("r1") = s;
        __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
    }
#endif
}

static _Noreturn void end(unsigned status)
{
#if BU_F439
    (void)status;
    for (;;)
    {
        if (BU_WATCHDOG)
        {
            REG(0x40003000) = 0xaaaa;
        }
        else
        {
            __asm__ volatile("wfi");
        }
    }
#else
    uint32_t args[2] = {0x20026, status};
    register unsigned r0 __asm__("r0") = 0x20;
    register uint32_t *r1 __asm__("r1") = args;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
    for (;;)
    {
    }
#endif
}

static void check(int yes, const char *why)
{
    if (!yes)
    {
        out(why);
        end(1);
    }
}

static void number(unsigned v)
{
    char b[12];
    unsigned i = 10;
    b[11] = 0;
    do
    {
        b[i--] = (char)('0' + v % 10);
        v /= 10;
    } while (v);
    out(b + i + 1);
}

void bu_fault_report(void)
{
    unsigned exception;
    __asm__ volatile("mrs %0,ipsr" : "=r"(exception));
    if (BU_GUARD_FAULT && exception == 4 && (REG(0xe000ed28) & 0x82) == 0x82 &&
        REG(0xe000ed34) == (uintptr_t)_stack_low - 4)
    {
        out("BU_GUARD_PASS\n");
        end(0);
    }
    out("BU_FAULT\n");
    end(1);
}

void bu_tick(void)
{
#if BU_FP
    volatile float x = 1.25f, y = 2.5f, z = x + y;
    check(z == 3.75f, "BU_CHECK:fp\n");
#endif
    ++ticks;
    __asm__ volatile("dsb sy" ::: "memory");
}

void bu_svc(void)
{
    ++svc_count;
    REG(0xe000ed04) = 1u << 26;
    __asm__ volatile("dsb sy\nisb" ::: "memory");
    while (ticks < svc_count)
    {
    } /* SysTick priority is higher; finite campaign timeout checks it. */
}

__attribute__((noinline)) static void leaf(unsigned n)
{
    volatile unsigned char local[64];
    for (unsigned i = 0; i < 64; i++)
    {
        local[i] = (unsigned char)(n + i);
    }
    __asm__ volatile("svc 0" ::: "memory");
    scratch_sink = local[n & 63];
}

__attribute__((noinline)) static void middle(unsigned n)
{
    volatile unsigned char local[48];
    for (unsigned i = 0; i < 48; i++)
    {
        local[i] = (unsigned char)(n - i);
    }
    leaf(n);
    scratch_sink ^= local[n & 31];
}

void bu_main(void)
{
    check(bu_stage == 2, "BU_CHECK:stage\n");
    check(bu_data == 0x12345678 && bu_bank == 0x87654321, "BU_CHECK:copy\n");
    check(!bu_bss && !bu_bank_bss, "BU_CHECK:zero\n");
    bu_stage = 3;
    /* No STM32 RCC/MMIO is executed on MPS2. F439 remains its cold HSI16 profile.
     */
    check(REG(0xe000ed08) == (uint32_t)(uintptr_t)&bu_vectors_target,
          "BU_CHECK:vtor\n");
#if BU_RAM_VECTORS
    for (unsigned i = 0; i < 107; i++)
    {
        ram_vectors[i] = bu_vectors_target[i];
    }
    REG(0xe000ed08) = (uint32_t)(uintptr_t)ram_vectors;
    __asm__ volatile("dsb sy\nisb" ::: "memory");
    check(REG(0xe000ed08) == (uint32_t)(uintptr_t)ram_vectors,
          "BU_CHECK:ram vectors\n");
#endif
    /*32-byte aligned no-access MPU region below reserved stack. */
    check(REG(0xe000ed90) & 0xff00, "BU_CHECK:mpu absent\n");
    REG(0xe000ed98) = 0;
    REG(0xe000ed9c) = (uint32_t)(uintptr_t)_stack_low - 32;
    REG(0xe000eda0) = (1u << 28) | (4u << 1) | 1;
    REG(0xe000ed94) = 5;
    REG(0xe000ed24) |= 1u << 16;
    __asm__ volatile("dsb sy\nisb" ::: "memory");
    if (BU_GUARD_FAULT)
    {
        if (!BU_MASK_GUARD)
        {
            __asm__ volatile("cpsie i" ::: "memory");
        }
        REG((uintptr_t)_stack_low - 4) = 7;
    }
    REG(0xe000ed1c) = (REG(0xe000ed1c) & 0x00ffffffu) | (0x80u << 24);
    REG(0xe000ed20) = (REG(0xe000ed20) & 0x00ffffffu) | (0x40u << 24);
    __asm__ volatile("cpsie i" ::: "memory");
    middle(7);
    __asm__ volatile("cpsid i" ::: "memory");
    check(svc_count == 1 && ticks == 1, "BU_CHECK:nesting\n");
    uintptr_t p = (uintptr_t)_stack_low;
    while (p < (uintptr_t)_stack_top && *(const unsigned char *)p == 0xa5)
    {
        ++p;
    }
    unsigned used = (unsigned)((uintptr_t)_stack_top - p);
    check(used >= 112 && used < 2048, "BU_CHECK:watermark\n");
    out("BU_STACK ");
    number(used);
    out("\n");
    if (BU_SOFTWARE_RESET && bu_retained != 0x42553237)
    {
        bu_retained = 0x42553237;
        REG(0xe000ed0c) = 0x05fa0004;
        __asm__ volatile("dsb sy" ::: "memory");
        for (;;)
        {
        }
    }
    check(!BU_SOFTWARE_RESET || bu_retained == 0x42553237,
          "BU_CHECK:retained\n");
#if BU_F439 && BU_WATCHDOG
    if (!(bu_reset_flags & (1u << 29)) || bu_retained != 0x57443237)
    {
        unsigned bound = 100000;
        REG(0x40023874) |= 1u;
        while (!(REG(0x40023874) & 2) && --bound)
        {
        }
        check(bound != 0, "BU_CHECK:LSI timeout\n");
        bound = 100000;
        while ((REG(0x4000300c) & 3) && --bound)
        {
        }
        check(bound != 0, "BU_CHECK:IWDG update timeout\n");
        REG(0x40003000) = 0x5555;
        REG(0x40003004) = 6;
        REG(0x40003008) = 511;
        bound = 100000;
        while ((REG(0x4000300c) & 3) && --bound)
        {
        }
        check(bound != 0, "BU_CHECK:IWDG settle timeout\n");
        bu_retained = 0x57443237;
        REG(0x40023874) |= 1u << 24;
        REG(0x40003000) = 0xcccc;
        REG(0x40003000) = 0xaaaa;
        __asm__ volatile("dsb sy" ::: "memory");
        for (;;)
        {
        }
    }
#endif
    bu_stage = 4;
    out("BU_TARGET_PASS copy2 zero2 nested2 retained stage\n");
}

void bu_returned(void)
{
    check(bu_stage == 5, "BU_CHECK:return\n");
    out("BU_RETURN_PASS\n");
    end(0);
}
