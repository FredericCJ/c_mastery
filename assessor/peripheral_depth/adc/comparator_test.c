#include "adc.h"
#include <stdio.h>
#define CHECK(x)                                              \
    do                                                        \
    {                                                         \
        if (!(x))                                             \
        {                                                     \
            printf("PA_COMP_CHECK:%u\n", (unsigned)__LINE__); \
            return 1;                                         \
        }                                                     \
    } while (0)

static struct
{
    uint32_t address, value;
} regs[12];

static unsigned n;

volatile uint32_t *pa_test_register(uint32_t a)
{
    for (unsigned i = 0; i < n; i++)
        if (regs[i].address == a)
            return &regs[i].value;
    regs[n].address = a;
    return &regs[n++].value;
}

#define R(a) (*pa_test_register(a))

int main(void)
{
    for (unsigned hyst = 0; hyst < 8; hyst++)
        for (unsigned wrap = 0; wrap < 2; wrap++)
        {
            struct pa_comparator s = {0};
            unsigned level = 88;
            uint32_t now = wrap ? UINT32_MAX - 2 : 0;
            CHECK(pa_g474_comp_open(&s, hyst) == PA_OK &&
                  R(0x40010200u) == ((hyst << 16) | 0x71u) &&
                  (R(0x48000000u) & 15) == 15);
            CHECK(pa_g474_comp_poll(&s, now, &level) == PA_BUSY && level == 88);
            CHECK(pa_g474_comp_poll(&s, now + 5, &level) == PA_BUSY &&
                  level == 88);
            CHECK(pa_g474_comp_poll(&s, now + 6, &level) == PA_OK &&
                  level == 0);
            R(0x40010200u) |= UINT32_C(1) << 30;
            CHECK(pa_g474_comp_poll(&s, now + 7, &level) == PA_OK &&
                  level == 1);
            CHECK(pa_g474_comp_close(&s) == PA_OK && !R(0x40010200u));
            CHECK(pa_g474_comp_poll(&s, now + 8, &level) == PA_STALE);
        }
    struct pa_comparator s = {0};
    R(0x40010200u) = UINT32_C(1) << 31;
    CHECK(pa_g474_comp_open(&s, 3) == PA_BUSY &&
          R(0x40010200u) == (UINT32_C(1) << 31));
    CHECK(pa_g474_comp_close(&s) == PA_BUSY);
    puts("PA_COMP_PASS");
    return 0;
}
