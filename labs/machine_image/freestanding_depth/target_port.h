#ifndef CMASTERY_FREE_TARGET_PORT_H
#define CMASTERY_FREE_TARGET_PORT_H
#include <stddef.h>
#include <stdint.h>

/* Supplied MPS2 AN386-only peripheral adapter. These are memory-mapped device
 * accesses in the declared emulator ABI, not ISO C address guarantees. */
static inline void fr_uart_init(void)
{
    *(volatile uint32_t *)0x40004010 = 217;
    *(volatile uint32_t *)0x40004008 = 1;
}

static inline int fr_uart_byte(unsigned char b)
{
    for (unsigned limit = 0; limit < 100000; ++limit)
    {
        if (!(*(volatile uint32_t *)0x40004004 & 1))
        {
            *(volatile uint32_t *)0x40004000 = b;
            return 0;
        }
    }
    return -1;
}

static inline long fr_semihost(unsigned op, const void *p)
{
    register unsigned r0 __asm__("r0") = op;
    register const void *r1 __asm__("r1") = p;
    __asm__ volatile("bkpt 0xab" : "+r"(r0), "+r"(r1)::"memory");
    return (long)r0;
}

static inline void fr_stop(unsigned code)
{
    unsigned a[2] = {0x20026, code};
    (void)fr_semihost(0x20, a);
    for (;;)
    {
        __asm__ volatile("wfi");
    }
}
#endif
