#include "target.h"

void mf_text(const char *s)
{
    register unsigned r0 __asm__("r0") = 4;
    register const char *r1 __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

void mf_number(uint32_t n)
{
    char text[11];
    for (unsigned i = 0; i < 8; i++)
    {
        unsigned digit = (n >> ((7 - i) * 4)) & 15u;
        text[i] = (char)(digit < 10 ? '0' + digit : 'a' + digit - 10);
    }
    text[8] = '\n';
    text[9] = 0;
    mf_text(text);
}

void mf_finish(unsigned code)
{
    uint32_t data[2] = {0x20026u, code};
    register unsigned r0 __asm__("r0") = 0x20;
    register const uint32_t *r1 __asm__("r1") = data;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
    for (;;)
        __asm__ volatile("wfi");
}

void mf_require(int condition, unsigned tag)
{
    if (!condition)
    {
        mf_text("FAIL MF ");
        mf_number(tag);
        mf_finish(1);
    }
}

void mf_trap(void)
{
    uint32_t ip, psp, msp;
    __asm__ volatile("mrs %0,ipsr\nmrs %1,psp\nmrs %2,msp"
                     : "=r"(ip), "=r"(psp), "=r"(msp));
    mf_text("FAIL MF fault IPSR/CFSR/HFSR/PSP/MSP\n");
    mf_number(ip);
    mf_number(*(volatile uint32_t *)0xe000ed28);
    mf_number(*(volatile uint32_t *)0xe000ed2c);
    mf_number(psp);
    mf_number(msp);
    mf_finish(2);
}
