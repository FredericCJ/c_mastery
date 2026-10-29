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

void ss_note(const char *s)
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

void ss_values(const char *s, unsigned a, unsigned b, unsigned c, unsigned d)
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
    text("SS_GUEST_FAULT\n");
    stop(42);
}

static volatile unsigned interrupts;
void ss_irq_old_event(void);

void xf_capture(uint32_t *sp, uint32_t er, unsigned irq, uint32_t msp)
{
    (void)sp;
    (void)er;
    (void)msp;
    if (irq != 16)
        xf_fault();
    ss_irq_old_event();
    interrupts++;
}

int ss_edges(void);
int ss_queue(void);
int ss_quiescence(void);
int ss_irq_prepare(void);
int ss_irq_verify(void);
int ss_mapped(void);
int ss_device(void);

void xf_target_main(void)
{
    int r = ss_edges();
    if (r)
        stop((unsigned)r);
    r = ss_queue();
    if (r)
        stop((unsigned)r);
    r = ss_quiescence();
    if (r)
        stop((unsigned)r);
    r = ss_device();
    if (r)
        stop((unsigned)r);
    r = ss_irq_prepare();
    if (r)
        stop((unsigned)r);
    /* The original imported IRQ0 wrapper enters xf_capture; foreground is masked. */
    *(volatile uint32_t *)0xe000e100u = 1;
    *(volatile uint32_t *)0xe000e280u = 1;
    __asm__ volatile("dsb sy\nisb sy\ncpsie i" ::: "memory");
    *(volatile uint32_t *)0xe000e200u = 1;
    __asm__ volatile("dsb sy\nisb sy\nnop\nnop\nnop\ncpsid i" ::: "memory");
    if (interrupts != 1)
    {
        text("SS_IRQ_ASSERT: one actual exception\n");
        stop(41);
    }
    r = ss_irq_verify();
    if (r)
        stop((unsigned)r);
    ss_values("irq_exception", interrupts, 16, 1, 0);
    r = ss_mapped();
    if (r)
        stop((unsigned)r);
    text("SS_GUEST_PASS CPU-IRQ-and-RAM-model no-physical-SPI\n");
    stop(0);
}
