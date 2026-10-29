#include "sync.h"
void su_note(const char *);
void su_values(const char *, unsigned, unsigned, unsigned, unsigned);
#define CHECK(x)                           \
    do                                     \
    {                                      \
        if (!(x))                          \
        {                                  \
            su_note("SU_MAP_ASSERT: " #x); \
            return 41;                     \
        }                                  \
    } while (0)

struct ep_registers
{
    volatile uint32_t sr, dr, brr, cr1, cr2, cr3, gtpr;
};
static struct ep_registers u;
static uint32_t addresses[32], values[32];
static unsigned used;

volatile uint32_t *pu_test_register(uint32_t a)
{
    for (unsigned i = 0; i < used; i++)
    {
        if (addresses[i] == a)
        {
            return values + i;
        }
    }
    if (used == 32)
    {
        return 0;
    }
    addresses[used] = a;
    return values + used++;
}

volatile uint32_t *su_test_register(uint32_t a)
{
    return pu_test_register(a);
}

struct ep_registers *pu_test_usart(void)
{
    return &u;
}

#define R(a) (*pu_test_register(a))

int su_mapped(void)
{
    struct mc_su s = {0};
    struct pu_io io = pu_f439_io();
    struct pu_clock c = {42000000, 1, 1, 1, 1, 0};
    R(0x40023800u) = 2;
    R(0x40023808u) = 0;
    CHECK(pu_open(&s.byte, io, &c, 115200, 0, 1) == PU_OK);
    CHECK(u.brr == 0x16d && u.cr1 == 0x200c);
    CHECK(su_configure(&s, io, 1, 0, 1) == PU_BUSY);
    io.write(io.context, PU_CR1, u.cr1 & ~12u);
    CHECK(su_configure(&s, io, 1, 0, 1) == PU_OK && u.cr2 == 0xd00 &&
          u.cr3 == 0);
    CHECK(su_enable(&s, io, 1, 1) == PU_OK && u.cr1 == 0x200c);
    R(0x40020c00u) = 0xffffffffu;
    R(0x40020c04u) = 0xffffffffu;
    R(0x40020c08u) = 0xffffffffu;
    R(0x40020c0cu) = 0xffffffffu;
    R(0x40020c24u) = 0xffffffffu;
    pu_f439_pins();
    su_f439_ck();
    CHECK(R(0x40020c00u) == 0xffeaffffu && R(0x40020c04u) == 0xfffff8ffu);
    CHECK(R(0x40020c08u) == 0xffc0ffffu && R(0x40020c0cu) == 0xffc4ffffu);
    CHECK(R(0x40020c24u) == 0xfffff777u && (R(0x40023830u) & 8));
    su_values("mapped", u.brr, u.cr2, R(0x40020c24u), R(0x40020c00u));
    return 0;
}
