#include <stdint.h>

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

void ia_note(const char *s)
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

void ia_values(const char *s, unsigned a, unsigned b, unsigned c, unsigned d)
{
    text("OBS ");
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

static void stop(unsigned n)
{
    uint32_t a[2] = {0x20026u, n};
    semi(0x20, a);
    for (;;)
    {
    }
}

void xf_fault(void)
{
    text("IA_GUEST_FAULT\n");
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

int ia_cases(unsigned);
int ia_mapped(void);

void xf_target_main(void)
{
    for (unsigned i = 0; i < 6; i++)
    {
        int r = ia_cases(i);
        if (r)
        {
            stop((unsigned)r);
        }
    }
    int r = ia_mapped();
    if (r)
    {
        stop((unsigned)r);
    }
    text("IA_GUEST_PASS CPU-and-RAM-model no-physical-I2C\n");
    stop(0);
}
