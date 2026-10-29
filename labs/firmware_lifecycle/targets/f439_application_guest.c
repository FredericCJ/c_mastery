#include "f439_update.h"
#ifndef APP_GENERATION
#define APP_GENERATION 2
#endif
static uint32_t control;
static unsigned starts;

static uint32_t rd(void *c, uint32_t a)
{
    (void)c;
    return a == 0x40023C10 ? control : 0;
}

static void wr(void *c, uint32_t a, uint32_t n)
{
    (void)c;
    if (a == 0x40023C10)
    {
        control = n;
        if (n & 0x10000u)
        {
            ++starts;
        }
    }
}

static void sync_io(void *c)
{
    (void)c;
}

static void fresh(void *c, uint32_t a, uint32_t n)
{
    (void)c;
    (void)a;
    (void)n;
}

static void output(const char *s)
{
    register uint32_t r0 __asm__("r0") = 4;
    register const char *r1 __asm__("r1") = s;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

_Noreturn void th_guest_report(uint32_t result, uint32_t generation)
{
    struct f4_selection old = {.sequence = 1,
                               .record = 0,
                               .image = {.core = 0,
                                         .base = 0x08080200,
                                         .generation = 1,
                                         .version = 1}},
                        trial = {.sequence = 2,
                                 .record = 0,
                                 .image = {.core = 0,
                                           .base = 0x080A0200,
                                           .generation = APP_GENERATION,
                                           .version = APP_GENERATION}};
    struct f4_writer writer = {0};
    struct fm_io io = {{0, rd, wr, sync_io}, fresh};
    enum fh_result z =
        f4_commit_begin(&writer, io, &old, &trial, generation, result);
    unsigned pass = z == FH_PENDING && starts == 1;
    output(
        pass
            ? "F4_CPU_PASS: actual finite application admitted by single-core commit gate\n"
            : "F4_CPU_BAD: wrong application result rejected\n");
    uint32_t code[2] = {0x20026u, pass ? 0 : 1};
    register uint32_t r0 __asm__("r0") = 0x20;
    register uint32_t *r1 __asm__("r1") = code;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
    for (;;)
    {
        __asm__ volatile("wfi");
    }
}
