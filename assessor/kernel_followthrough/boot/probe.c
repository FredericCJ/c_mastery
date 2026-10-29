#include "probe.h"

static inline void out(unsigned short port, unsigned char value)
{
    __asm__ volatile("outb %0,%1" ::"a"(value), "Nd"(port));
}

static inline unsigned char in(unsigned short port)
{
    unsigned char value;
    __asm__ volatile("inb %1,%0" : "=a"(value) : "Nd"(port));
    return value;
}

static void serial(void)
{
    out(0x3f9, 0);
    out(0x3fb, 0x80);
    out(0x3f8, 1);
    out(0x3f9, 0);
    out(0x3fb, 3);
    out(0x3fa, 0xc7);
    out(0x3fc, 0x0b);
}

static void text(const char *s)
{
    for (unsigned i = 0; s[i] && i < 160; ++i)
    {
        unsigned n = 0;
        while (!(in(0x3fd) & 32) && n < 100000)
        {
            ++n;
        }
        if (n == 100000)
        {
            return;
        }
        out(0x3f8, (unsigned char)s[i]);
    }
}

static void hex(uint64_t n)
{
    char b[18];
    b[16] = '\n';
    b[17] = 0;
    for (unsigned i = 0; i < 16; ++i)
    {
        b[i] = "0123456789abcdef"[(n >> (60 - 4 * i)) & 15];
    }
    text(b);
}

static _Noreturn void stop(unsigned char status)
{
    __asm__ volatile("outl %0,%1" ::"a"((unsigned)status),
                     "Nd"((unsigned short)0xf4));
    for (;;)
    {
        __asm__ volatile("cli; hlt");
    }
}

void kf_boot_check(uint32_t magic, uint32_t info)
{
    uint64_t flags, cr0, cr3, cr4;
    uint32_t lo, hi;
    unsigned short cs;
    __asm__ volatile("pushfq; popq %0" : "=r"(flags));
    __asm__ volatile("mov %%cr0,%0" : "=r"(cr0));
    __asm__ volatile("mov %%cr3,%0" : "=r"(cr3));
    __asm__ volatile("mov %%cr4,%0" : "=r"(cr4));
    __asm__ volatile("mov %%cs,%0" : "=r"(cs));
    __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(0xc0000080));
    serial();
    text("KF:C_ENTRY\nmagic=");
    hex(magic);
    text("info=");
    hex(info);
    text("entry_rsp=");
    hex(kf_entry_rsp);
    text("cr3=");
    hex(cr3);
    text("cr0=");
    hex(cr0);
    text("cr4=");
    hex(cr4);
    text("efer=");
    hex(((uint64_t)hi << 32) | lo);
    if (magic != 0x36d76289 || !info || (info & 7))
    {
        text("KF:FAIL:BOOT_ARGS\n");
        stop(0x11);
    }
    if ((kf_entry_rsp & 15) != 8)
    {
        text("KF:FAIL:STACK_ALIGNMENT\n");
        stop(0x11);
    }
    if ((flags & (1u << 10)) || cs != 8 || (cr0 & 0x80010001) != 0x80010001 ||
        !(cr4 & 32) || (lo & 0xd00) != 0xd00 || cr3 != (uintptr_t)boot_pml4)
    {
        text("KF:FAIL:MODE\n");
        stop(0x11);
    }
    if ((boot_pml4[0] & ~UINT64_C(0x60)) != ((uintptr_t)boot_pdpt | 3))
    {
        text("KF:FAIL:PML4\n");
        stop(0x11);
    }
    for (unsigned i = 0; i < 4; ++i)
    {
        if ((boot_pdpt[i] & ~UINT64_C(0x60)) !=
            ((uintptr_t)boot_pd + 4096u * i + 3))
        {
            text("KF:FAIL:PDPT\n");
            stop(0x11);
        }
    }
    for (unsigned i = 0; i < 2048; ++i)
    {
        if ((boot_pd[i] & ~UINT64_C(0x60)) != ((uint64_t)i * 0x200000 + 0x83))
        {
            text("KF:FAIL:PD\n");
            stop(0x11);
        }
    }
    text("KF:PASS:2048_PAGES_4GIB_SUPERVISOR\nKF:PASS:BOOT_ONLY_C17\n");
    stop(0x10);
}
