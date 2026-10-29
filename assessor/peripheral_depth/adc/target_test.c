#include "adc.h"
#include <stdio.h>
#define CHECK(x)                                                \
    do                                                          \
    {                                                           \
        if (!(x))                                               \
        {                                                       \
            printf("PA_TARGET_CHECK:%u\n", (unsigned)__LINE__); \
            return 1;                                           \
        }                                                       \
    } while (0)

static struct
{
    uint32_t address, value;
} regs[48];

static unsigned n, reset_count, resetting;

volatile uint32_t *pa_test_register(uint32_t a)
{
    for (unsigned i = 0; i < n; i++)
    {
        if (regs[i].address != 0x40023820u)
            continue;
        if (!(regs[i].value & 2u))
            resetting = 0;
        else if (!resetting)
        {
            resetting = 1;
            reset_count++;
            for (unsigned j = 0; j < n; j++)
                if (regs[j].address >= 0x40000400u &&
                    regs[j].address < 0x40000460u)
                    regs[j].value = 0;
        }
    }
    for (unsigned i = 0; i < n; i++)
        if (regs[i].address == a)
            return &regs[i].value;
    regs[n].address = a;
    return &regs[n++].value;
}

#define R(a) (*pa_test_register(a))

int main(void)
{
    pa_f439_pins();
    CHECK((R(0x40020000u) & 15) == 15 && (R(0x40023844u) & 256));
    struct pa_io io = pa_f439_io();
    struct pa_adc s = {0};
    struct pu_clock clock = {64000000, 1, 1, 2, 2, 0};
    struct pa_config cfg = {4, 2, 1};
    CHECK(pa_open(&s, io, &clock, &cfg, 0) == PA_OK && R(0x40012010u) == 36 &&
          R(0x4001202cu) == 1048576 && R(0x40012034u) == 32);
    R(0x4001204cu) = 123;
    CHECK(io.read(0, PA_DR) == 123);
    R(0x40000408u) = 7; /* Inherited slave/external-clock mode. */
    R(0x4000040cu) = 1; /* Inherited update interrupt enable. */
    CHECK(pa_f439_timer(63, 999, 0) == PA_OK && R(0x40000404u) == 32 &&
          R(0x40000428u) == 63 && R(0x4000042cu) == 999 && !R(0x40000400u));
    CHECK(reset_count == 1 && !R(0x40000408u) && !R(0x4000040cu));
    R(0x40000414u) = 0;
    CHECK(pa_f439_timer(63, 999, 1) == PA_OK && R(0x40000400u) == 1 &&
          !R(0x40000414u));
    CHECK(pa_f439_timer(63, 998, 1) == PA_INVALID);
    pa_f439_dac_open();
    CHECK(R(0x40007400u) == 0x3d && (R(0x40020000u) & 0x300) == 0x300);
    CHECK(pa_f439_dac_write(0xabc) == PA_OK && R(0x40007408u) == 0xabc &&
          R(0x40007404u) == 1);
    CHECK(pa_f439_dac_write(4096) == PA_INVALID && R(0x40007408u) == 0xabc);
    R(0x4000742cu) = 0xfabc;
    CHECK(pa_f439_dac_read() == 0xabc);
    puts("PA_TARGET_PASS");
    return 0;
}
