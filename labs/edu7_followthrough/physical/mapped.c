#include "physical.h"
int puts(const char *);
#define CHECK(x)                   \
    do                             \
    {                              \
        if (!(x))                  \
        {                          \
            puts("EP_ASSERT:" #x); \
            return 41;             \
        }                          \
    } while (0)

struct ep_registers
{
    volatile uint32_t sr, dr, brr, cr1, cr2, cr3, gtpr;
};

static uint32_t rcc[64], gpioa[16], gpiod[16], dma[64], uart2[7];
static struct ep_registers uart3;
static unsigned bad, barriers;

volatile uint32_t *ep_register(uint32_t a)
{
    if (a >= 0x40023800u && a < 0x40023900u && !(a & 3))
    {
        return &rcc[(a - 0x40023800u) / 4];
    }
    if (a >= 0x40020000u && a < 0x40020040u && !(a & 3))
    {
        return &gpioa[(a - 0x40020000u) / 4];
    }
    if (a >= 0x40020c00u && a < 0x40020c40u && !(a & 3))
    {
        return &gpiod[(a - 0x40020c00u) / 4];
    }
    if (a >= 0x40004400u && a < 0x4000441cu && !(a & 3))
    {
        return &uart2[(a - 0x40004400u) / 4];
    }
    if (a >= 0x40026000u && a < 0x40026100u && !(a & 3))
    {
        return &dma[(a - 0x40026000u) / 4];
    }
    bad++;
    return &rcc[63];
}

volatile uint32_t *pu_test_register(uint32_t a)
{
    return ep_register(a);
}

struct ep_registers *pu_test_usart(void)
{
    return &uart3;
}

volatile uint32_t *r5_test_register(uint32_t a)
{
    return ep_register(a);
}

void ep_barrier(void)
{
    barriers++;
}

void r5_test_barrier(void)
{
    ep_barrier();
}

int ep_mapped(void)
{
    struct ep_tx a = {0}, b = {0};
    struct pu_clock c = {16000000, 1, 1, 1, 1, 0};
    rcc[0] = 2;
    CHECK(ep_f439_open_port(&a, &c, 115200, R5_THVD1400, 0, 0) == EP_OK);
    CHECK(ep_f439_open_port(&b, &c, 115200, R5_THVD1400, 1, 0) == EP_OK);
    CHECK((rcc[12] & 9u) == 9u && (rcc[16] & (3u << 17)) == (3u << 17));
    CHECK((gpioa[8] & 0xff00u) == 0x7700u && (gpiod[9] & 255u) == 0x77u);
    CHECK((gpioa[0] & 252u) == 164u && ((gpiod[0] >> 16) & 63u) == 26u);
    CHECK(gpioa[6] == (1u << 17) && gpiod[6] == (1u << 26) && uart2[2] == 139 &&
          uart3.brr == 139);
    uint8_t x = 17, y = 51;
    CHECK(ep_send(&a, 1, &x, 1, 0, 10000) == EP_OK &&
          ep_send(&b, 1, &y, 1, 0, 10000) == EP_OK);
    CHECK(gpioa[6] == 2 && gpiod[6] == 1024);
    uart2[0] = PU_TXE | PU_TC;
    uart3.sr = PU_TXE | PU_TC;
    CHECK(ep_step(&a, 1, 199) == EP_BUSY && ep_step(&b, 1, 199) == EP_BUSY &&
          uart3.dr == 0 && uart2[1] == 0);
    CHECK(ep_step(&a, 1, 200) == EP_BUSY && ep_step(&b, 1, 200) == EP_BUSY &&
          uart3.dr == 17 && uart2[1] == 51);
    /* Distinct two-wire segments may drive concurrently. Same-segment overlap
       requires independent peer scheduling, not this numerical register model. */
    uart3.sr = PU_TC;
    CHECK(ep_step(&a, 1, 287) == EP_BUSY && gpiod[6] == (1u << 26) &&
          gpioa[6] == 2);
    uart2[0] = PU_TC;
    CHECK(ep_step(&b, 1, 288) == EP_BUSY && gpioa[6] == (1u << 17));
    CHECK(ep_step(&a, 1, 487) == EP_OK && ep_step(&b, 1, 488) == EP_OK &&
          !bad && barriers == 8);
    puts("EP_PORT_VALUES 139 17 51 2");
    puts("EP_PORT_PASS two_distinct_register_banks_not_two_physical_segments");
    return 0;
}
