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

void su_note(const char *s)
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

void su_values(const char *s, unsigned a, unsigned b, unsigned c, unsigned d)
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

static void stop(unsigned code)
{
    uint32_t args[2] = {0x20026u, code};
    semi(0x20, args);
    for (;;)
    {
    }
}

void xf_fault(void)
{
    text("SU_GUEST_FAULT\n");
    stop(42);
}

/* Imported reset/vector code retains these entries; IRQs stay masked here. */
void xf_capture(uint32_t *sp, uint32_t er, unsigned irq, uint32_t msp)
{
    (void)sp;
    (void)er;
    (void)irq;
    (void)msp;
    xf_fault();
}

int su_cases(unsigned);
int su_mapped(void);

void xf_target_main(void)
{
    for (unsigned i = 0; i < 4; i++)
    {
        int r = su_cases(i);
        if (r)
        {
            stop((unsigned)r);
        }
    }
    int r = su_mapped();
    if (r)
    {
        stop((unsigned)r);
    }
    text("SU_GUEST_PASS model-in-RAM no-physical-USART\n");
    stop(0);
}
