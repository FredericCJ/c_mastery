#include "ledger.h"
int mf_run(unsigned);
void mf_original_target_main(void);

static void write_text(const char *s)
{
    register unsigned r0 __asm__("r0") = 4;
    register const char *r1 __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

void mf_emit(const char *s, const uint32_t *p, size_t n)
{
    (void)s;
    (void)p;
    (void)n;
}

int mf_fail(const char *s, unsigned line)
{
    (void)line;
    write_text("MF_ASSERT:");
    write_text(s);
    write_text("\n");
    return 41;
}

void xf_target_main(void)
{
    unsigned failed = 0;
    for (unsigned i = 0; i < 4; ++i)
        failed |= (unsigned)mf_run(i);
    if (failed)
    {
        unsigned args[] = {0x20026, 1};
        write_text("MF_TARGET_FAIL\n");
        register unsigned r0 __asm__("r0") = 0x20;
        register unsigned *r1 __asm__("r1") = args;
        __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
        for (;;)
        {
        }
    }
    write_text("MF_TARGET_PREFLIGHT_PASS\n");
    mf_original_target_main();
}
