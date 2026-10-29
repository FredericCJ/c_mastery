#include "order.h"
#include <stdint.h>
#define REG(a) (*(volatile uint32_t *)(uintptr_t)(a))

void xd_print(const char *s)
{
    register unsigned r0 __asm__("r0") = 4;
    register const char *r1 __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

static void hex(uint32_t x)
{
    char b[10];
    for (unsigned i = 0; i < 8; ++i)
    {
        b[i] = "0123456789abcdef"[(x >> (28 - 4 * i)) & 15];
    }
    b[8] = ' ';
    b[9] = 0;
    xd_print(b);
}

__attribute__((noreturn)) static void finish(unsigned code)
{
    unsigned args[2] = {0x20026, code};
    register unsigned r0 __asm__("r0") = 0x20;
    register unsigned *r1 __asm__("r1") = args;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
    for (;;)
    {
    }
}

void xf_fault(void)
{
    xd_print("XD_FAULT\n");
    finish(2);
}

static uint32_t now(void)
{
    return UINT32_MAX - REG(0x40001004);
}

static volatile uint32_t payload;
int xd_composition(void);

void xf_target_main(void)
{
    if (xd_composition())
    {
        finish(1);
    }
    REG(0x40001000) = 0;
    REG(0x40001008) = UINT32_MAX;
    REG(0x40001000) = 1;
    xd_compiler();
    xd_dmb();
    for (unsigned block = 0; block < 31; ++block)
    {
        uint32_t observed[2];
        for (unsigned turn = 0; turn < 2; ++turn)
        {
            unsigned policy = turn ^ (block & 1);
            uint32_t begin = now();
            for (unsigned i = 0; i < 128; ++i)
            {
                payload = i;
                if (policy)
                {
                    xd_double_dsb();
                }
                else
                {
                    xd_dsb();
                }
            }
            observed[policy] = now() - begin;
        }
        xd_print("XD_TIME ");
        hex(block);
        hex(observed[0]);
        hex(observed[1]);
        xd_print("\n");
    }
    xd_print("XD_TARGET_PASS blocks=31 operations=128 virtual_timer=25MHz\n");
    finish(0);
}
