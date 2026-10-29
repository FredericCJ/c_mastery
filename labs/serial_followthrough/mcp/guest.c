#include "mcp.h"

static void semi(uint32_t op, const void *p)
{
    register uint32_t a __asm__("r0") = op;
    register const void *b __asm__("r1") = p;
    __asm__ volatile("bkpt 0xab" : "+r"(a) : "r"(b) : "memory");
}

static void text(const char *s)
{
    semi(4, s);
}

void mc_note(const char *s)
{
    text(s);
    text("\n");
}

static void number(unsigned n)
{
    char b[12];
    unsigned i = 11;
    b[i] = 0;
    do
    {
        b[--i] = (char)('0' + n % 10);
        n /= 10;
    } while (n);
    text(b + i);
}

void mc_values(const char *s, unsigned a, unsigned b, unsigned c, unsigned d)
{
    text("OBS mcp_");
    text(s);
    text(" ");
    number(a);
    text(" ");
    number(b);
    text(" ");
    number(c);
    text(" ");
    number(d);
    text("\n");
}

static void stop(unsigned c)
{
    uint32_t p[] = {0x20026u, c};
    semi(0x20, p);
    for (;;)
    {
    }
}

void xf_fault(void)
{
    text("MCP_GUEST_FAULT\n");
    stop(42);
}

void xf_capture(uint32_t *sp, uint32_t er, unsigned irq, uint32_t msp)
{
    (void)sp;
    (void)er;
    (void)irq;
    (void)msp;
    xf_fault();
}

void xf_target_main(void)
{
    int (*t[])(void) = {mc_test_layout,   mc_test_init,    mc_test_failure,
                        mc_test_transfer, mc_test_receive, mc_test_bench,
                        mc_test_spi,      mc_test_atomic};
    for (unsigned i = 0; i < 8; i++)
        if (t[i]())
            stop(41);
    text("MCP_GUEST_PASS actual-Thumb controller-model no-physical-CAN\n");
    stop(0);
}
