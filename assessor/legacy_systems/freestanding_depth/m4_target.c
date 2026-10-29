#include "services.h"
#include "target_port.h"
#include "division_cases.h"
static unsigned char arena[32], snapshot[32];
static int enabled;

static struct fr_progress output(void *context, const unsigned char *b,
                                 size_t n)
{
    (void)context;
    size_t count = n < 2 ? n : 2;
    for (size_t i = 0; i < count; ++i)
    {
#if FR_TRANSPORT_SEMI
        if (!enabled)
            return (struct fr_progress){FR_IO, i};
        fr_semihost(3, b + i);
#else
        if (fr_uart_byte(b[i]))
            return (struct fr_progress){FR_IO, i};
#endif
    }
    return (struct fr_progress){FR_OK, count};
}

static enum fr_status debugger_service(void)
{
    if (!enabled)
        return FR_UNSUPPORTED;
    return FR_OK;
}

static void finish(const char *message, unsigned code)
{
    struct fr_service s;
    size_t n = 0;
    while (message[n])
        ++n;
    if (fr_service_init(&s, output, 0) != FR_OK ||
        fr_service_ready(&s) != FR_OK ||
        fr_service_begin(&s, (const unsigned char *)message, n) != FR_OK)
        code = 9;
    for (unsigned i = 0; i < 24 && s.active; ++i)
        if (fr_service_pump(&s) == FR_IO)
        {
            code = 10;
            break;
        }
#if FR_TRANSPORT_SEMI
    fr_stop(code);
#else
    (void)code;
    for (;;)
        __asm__ volatile("wfi");
#endif
}

void fr_fault(void)
{
    finish("FREE-M4:FAULT\n", 2);
}

void target_main(void)
{
    enabled = FR_TRANSPORT_SEMI;
    fr_uart_init();
    if (debugger_service() != (enabled ? FR_OK : FR_UNSUPPORTED))
        fr_fault();
    unsigned count = 0;
    for (size_t from = 0; from <= 16; ++from)
        for (size_t to = 0; to <= 16; ++to)
            for (size_t n = 0; n <= 16; ++n)
            {
                if (n > 16 - from || n > 16 - to)
                    continue;
                for (unsigned i = 0; i < 32; ++i)
                {
                    arena[i] = (unsigned char)i;
                    snapshot[i] = (unsigned char)i;
                }
                if (fr_arena_move(arena + 8, 16, to, from, n) != FR_OK)
                    fr_fault();
                for (unsigned i = 0; i < 32; ++i)
                    if (arena[i] != (i >= 8 + to && i < 8 + to + n
                                         ? snapshot[8 + from + i - (8 + to)]
                                         : snapshot[i]))
                        fr_fault();
                ++count;
            }
    if (count != 1785 || fr_arena_move(arena, 16, SIZE_MAX, 0, 1) != FR_RANGE)
        fr_fault();
    for (size_t i = 0; i < sizeof cases / sizeof cases[0]; ++i)
    {
        volatile uint64_t n = cases[i][0], d = cases[i][1];
        uint64_t q = n / d, r = n % d;
        if (q != cases[i][2] || r != cases[i][3])
            fr_fault();
    }
    /* Direct ABI bridge test d0 is distinct from undefined C n/0. */
    extern int fr_test_divzero(void);
    if (fr_test_divzero())
        fr_fault();
    finish("FREE-M4:1785:DIV:PASS\n", 0);
}
