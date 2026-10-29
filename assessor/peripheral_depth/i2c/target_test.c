#include "i2c.h"
#include <stdio.h>
#define CHECK(x)                                                \
    do                                                          \
    {                                                           \
        if (!(x))                                               \
        {                                                       \
            printf("PI_TARGET_CHECK:%u\n", (unsigned)__LINE__); \
            return 1;                                           \
        }                                                       \
    } while (0)
uint32_t pi_test_primask;

static struct
{
    uint32_t address, value;
} regs[40];

static unsigned n;

volatile uint32_t *pi_test_register(uint32_t a)
{
    for (unsigned i = 0; i < n; i++)
        if (regs[i].address == a)
            return &regs[i].value;
    regs[n].address = a;
    return &regs[n++].value;
}

#define R(a) (*pi_test_register(a))

int main(void)
{
    struct pi_i2c s = {0};
    struct pu_clock clock = {16000000, 1, 1, 1, 1, 0};
    uint8_t out = 77;
    pi_f439_pins();
    CHECK((R(0x40020424u) & 255u) == 0x44u &&
          (R(0x40020404u) & 0x300u) == 0x300u &&
          (R(0x40020400u) & 0xf0000u) == 0xa0000u);
    CHECK(pi_f439_open(&s, &clock) == PI_OK && R(0x4000541cu) == 100 &&
          R(0x40005420u) == 17 && R(0x40005408u) == 16384);
    pi_test_primask = 1;
    CHECK(pi_f439_begin(&s, 1, 1, 0x52, 0x10, 1, &out, 0, 100) == PI_OK &&
          pi_test_primask == 1);
    R(0x40005414u) = PI_SB;
    R(0x40005400u) &= ~PI_START;
    pi_test_primask = 0;
    CHECK(pi_f439_step(&s, 1, 1) == PI_BUSY && R(0x40005410u) == 0xa4 &&
          pi_test_primask == 0);
    CHECK(pi_f439_recovery_prepare(&s, 1, 1) == PI_FORBIDDEN);
    CHECK(pi_f439_cancel(&s, 1) == PI_BUSY);
    CHECK(pi_f439_recovery_prepare(&s, 1, 0) == PI_OK && !s.ready && !s.phase &&
          (R(0x40020400u) & 0xf0000u) == 0x50000u);
    struct pi_lines lines = pi_f439_lines();
    lines.release(0, PI_SCL, 0);
    CHECK(R(0x40020418u) == 1u << 24);
    lines.release(0, PI_SDA, 1);
    CHECK(R(0x40020418u) == 1u << 9);
    R(0x40020410u) = 0x100u;
    CHECK(lines.read(0, PI_SCL) == 1 && lines.read(0, PI_SDA) == 0);
    R(0x40005418u) = 0;
    R(0x40005400u) = 0;
    CHECK(pi_f439_open(&s, &clock) == PI_OK &&
          (R(0x40020400u) & 0xf0000u) == 0xa0000u);
    puts("PI_TARGET_PASS");
    return 0;
}
