#include <stdint.h>

static void semi(uint32_t op, const void *p)
{
    register uint32_t a __asm__("r0") = op;
    register const void *b __asm__("r1") = p;
    __asm__ volatile("bkpt 0xab" : "+r"(a) : "r"(b) : "memory");
}

void en_note(const char *s)
{
    semi(4, s);
    semi(4, "\n");
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
    en_note("EN_GUEST_FAULT");
    stop(42);
}

void xf_capture(uint32_t *a, uint32_t b, unsigned c, uint32_t d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    xf_fault();
}

int en_initialization(void);
int en_transmit(void);
int en_receive(void);
int en_recovery(void);
int en_drop_status(void);
int en_boundaries(void);
int en_failures(void);
int en_mapped(void);
int en_chain(void);
int en_publication(void);
int en_visibility(void);
int en_consumer(void);
int en_order(void);

void xf_target_main(void)
{
    int (*const tests[])(void) = {
        en_initialization, en_transmit, en_receive, en_recovery, en_drop_status,
        en_boundaries,     en_failures, en_mapped,  en_chain,    en_publication,
        en_visibility,     en_consumer, en_order};
    for (unsigned i = 0; i < 13; i++)
    {
        int r = tests[i]();
        if (r)
        {
            stop((unsigned)r);
        }
    }
    en_note(
        "EN_GUEST_PASS actual-Thumb original-RAM-device-effects no-physical-Ethernet");
    stop(0);
}
