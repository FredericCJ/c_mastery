#include "uart.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x)                                             \
    do                                                       \
    {                                                        \
        if (!(x))                                            \
        {                                                    \
            printf("PU_TARGET_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                        \
        }                                                    \
    } while (0)

struct ep_registers
{
    volatile uint32_t sr, dr, brr, cr1, cr2, cr3, gtpr;
};
static struct ep_registers u;
static uint32_t addresses[32], values[32];
static unsigned count;

volatile uint32_t *pu_test_register(uint32_t a)
{
    for (unsigned i = 0; i < count; i++)
    {
        if (addresses[i] == a)
        {
            return &values[i];
        }
    }
    if (count == 32)
    {
        abort();
    }
    addresses[count] = a;
    return &values[count++];
}

struct ep_registers *pu_test_usart(void)
{
    return &u;
}

#define R(a) (*pu_test_register(a))

int main(void)
{
    struct pu_io io = pu_f439_io();
    CHECK(io.read && io.write);
    R(0x40023808) = 0;
    R(0x40023800) = 2;
    CHECK(io.read(0, PU_SOURCE) == 1);
    R(0x40023808) = 8;
    CHECK(io.read(0, PU_SOURCE) == 0);
    R(0x40023800) |= 1u << 25;
    CHECK(io.read(0, PU_SOURCE) == 1);
    R(0x40023808) = 12;
    CHECK(io.read(0, PU_SOURCE) == 0);
    R(0x40023808) = 0;
    struct pu_driver d = {0};
    struct pu_clock c = {16000000, 1, 1, 1, 1, 0};
    CHECK(pu_open(&d, io, &c, 115200, 1, 2) == PU_OK && u.brr == 275 &&
          u.cr1 == 0xa00c && u.cr2 == 0 && u.cr3 == 0);
    CHECK(R(0x40023840) == 0x40000 && R(0x40023820) == 0);
    R(0x40020c00) = 0xffffffffu;
    R(0x40020c04) = 0xffffffffu;
    R(0x40020c08) = 0xffffffffu;
    R(0x40020c0c) = 0xffffffffu;
    R(0x40020c24) = 0xffffffffu;
    pu_f439_pins();
    CHECK(R(0x40020c00) == 0xfffaffffu && R(0x40020c04) == 0xfffffcffu &&
          R(0x40020c08) == 0xfff0ffffu && R(0x40020c0c) == 0xfff4ffffu &&
          R(0x40020c24) == 0xffffff77u);
    CHECK(pu_f439_dma_submit(&d, 1, 1, (const uint8_t *)(uintptr_t)0x10000000,
                             1, 0, 20) == PU_INVALID);
    CHECK(pu_f439_dma_submit(&d, 1, 1, (const uint8_t *)(uintptr_t)0x2002ffff,
                             2, 0, 20) == PU_INVALID);
    CHECK(pu_f439_dma_submit(&d, 1, 1, (const uint8_t *)(uintptr_t)0x20000000,
                             3, 0, 20) == PU_OK);
    CHECK(R(0x40026058) == 0x08000441 && R(0x40026060) == 0x40004804 &&
          R(0x40026064) == 0x20000000 && R(0x4002605c) == 3 &&
          R(0x4002606c) == 0 && R(0x40026008) == 0x0f400000 && u.sr == 0x320 &&
          u.cr3 == 128);
    R(0x40026000) = 0x08000000;
    CHECK(io.read(0, PU_DMA_FLAGS) == 1);
    R(0x40026000) = 0x03400000;
    CHECK(io.read(0, PU_DMA_FLAGS) == 2);
    CHECK(pu_cancel(&d, io, 1) == PU_OK && d.memory_owned &&
          !(R(0x40026058) & 1));
    struct pu_rx q = {0};
    q.backpressure = 1;
    q.reserve = 1;
    pu_f439_rts_init();
    CHECK(R(0x40020c18) == 0x1000 &&
          (R(0x40020c00) & 0x03000000) == 0x01000000);
    CHECK(pu_f439_rts(&q) == PU_OK && R(0x40020c18) == 0x10000000);
    CHECK(pu_receive(&q, PU_RXNE, 0) == PU_OK);
    CHECK(pu_f439_rts(&q) == PU_OK && R(0x40020c18) == 0x1000);
    puts("PU_TARGET_PASS mapped register writes only; no hardware observation");
    return 0;
}
