#include "checks.h"

/*
 * Cortex-M semihosting under QEMU: `bkpt 0xab` with the operation number in
 * r0 and a parameter in r1. Freestanding: no C library is used or linked.
 */
enum
{
    SEMIHOST_WRITE0 = 4, /* r1: NUL-terminated string */
    SEMIHOST_REPORT_EXCEPTION = 0x20, /* r1: pointer to {reason, code} */
    SEMIHOST_APPLICATION_EXIT = 0x20026,
    EXIT_PASS = 0,
    EXIT_CHECK_FAILED = 1,
    EXIT_FAULT = 2
};

static void emit(const char *s)
{
    register unsigned r0 __asm__("r0") = SEMIHOST_WRITE0;
    register const char *r1 __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(r0), "+r"(r1)::"memory");
}

/* Ends the emulation with exit status `rc`; never returns. */
_Noreturn static void stop(unsigned rc)
{
    unsigned args[2] = {SEMIHOST_APPLICATION_EXIT, rc};
    register unsigned r0 __asm__("r0") = SEMIHOST_REPORT_EXCEPTION;
    register unsigned *r1 __asm__("r1") = args;
    __asm__ volatile("bkpt 0xab" : "+r"(r0), "+r"(r1)::"memory");
    for (;;)
    {
    }
}

/* Every unused vector in startup.S points here. */
void p1_fault(void)
{
    emit("P1_TARGET_FAULT\n");
    stop(EXIT_FAULT);
}

void p1_target_main(void)
{
    if (p1_checks())
    {
        emit("P1_TARGET_FAIL\n");
        stop(EXIT_CHECK_FAILED);
    }
    emit("P1_TARGET_PASS traces=6561 memory_core explicit_runtime\n");
    stop(EXIT_PASS);
}
