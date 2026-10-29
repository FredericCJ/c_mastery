#include "spi.h"
#include <stdio.h>
#define CHECK(x)                                                \
    do                                                          \
    {                                                           \
        if (!(x))                                               \
        {                                                       \
            printf("PS_TARGET_CHECK:%u\n", (unsigned)__LINE__); \
            return 1;                                           \
        }                                                       \
    } while (0)

static struct
{
    uint32_t address, value;
} regs[64];

static unsigned count;

volatile uint32_t *ps_test_register(uint32_t address)
{
    for (unsigned i = 0; i < count; i++)
        if (regs[i].address == address)
            return &regs[i].value;
    regs[count].address = address;
    return &regs[count++].value;
}

#define R(a) (*ps_test_register(a))

int main(void)
{
    struct ps_io io = ps_f439_io();
    struct ps_spi s = {0};
    struct pu_clock c = {16000000, 1, 1, 1, 1, 0};
    ps_f439_pins();
    CHECK((R(0x40020020u) & 0xfff00000u) == 0x55500000u);
    CHECK((R(0x40020000u) & 0xfc00u) == 0xa800u);
    CHECK(R(0x40020c18u) == 1u << 14);
    CHECK(ps_open(&s, io, &c, 3, 3) == PS_OK && R(0x40013000u) == 0x35fu);
    io.write(0, PS_DMA_CLEAR, 1);
    CHECK(R(0x40026408u) == 0x0f40003du);
    io.write(0, PS_RX_ADDR, 0x20000000u);
    io.write(0, PS_TX_ADDR, 0x20000020u);
    io.write(0, PS_RX_COUNT, 3);
    io.write(0, PS_TX_COUNT, 3);
    io.write(0, PS_RX_START, 1);
    io.write(0, PS_TX_START, 1);
    CHECK(R(0x40026410u) == 0x06002c01u && R(0x40026458u) == 0x06002c41u &&
          R(0x40026418u) == 0x4001300cu && R(0x40026460u) == 0x4001300cu);
    CHECK(R(0x4002641cu) == 0x20000000u && R(0x40026464u) == 0x20000020u);
    R(0x40026400u) = 1u << 5 | 1u << 27;
    CHECK(io.read(0, PS_RX_FLAGS) == 1 && io.read(0, PS_TX_FLAGS) == 1);
    R(0x40026400u) = 1u << 3 | 1u << 25;
    CHECK(io.read(0, PS_RX_FLAGS) == 2 && io.read(0, PS_TX_FLAGS) == 2);
    io.write(0, PS_DMA_REQUESTS, 3);
    CHECK(R(0x40013004u) == 3);
    io.write(0, PS_TX_EN, 0);
    io.write(0, PS_RX_EN, 0);
    CHECK(!(R(0x40026410u) & 1u) && !(R(0x40026458u) & 1u));
    CHECK(ps_f439_begin_dma(&s, 1, 1, (void *)(uintptr_t)0x10000000u,
                            (void *)(uintptr_t)0x20000000u, 3, 0, 100, 1,
                            1) == PS_INVALID);
    CHECK(ps_f439_begin_dma(&s, 1, 1, (void *)(uintptr_t)0x20000000u,
                            (void *)(uintptr_t)0x20000001u, 3, 0, 100, 1,
                            1) == PS_INVALID);
    CHECK(ps_f439_begin_dma(&s, 1, 1, (void *)(uintptr_t)0x2002fffeu,
                            (void *)(uintptr_t)0x20000000u, 3, 0, 100, 1,
                            1) == PS_INVALID);
    R(0x40013008u) = PS_TXE;
    CHECK(ps_f439_begin_dma(&s, 1, 1, (void *)(uintptr_t)0x20000000u,
                            (void *)(uintptr_t)0x20000020u, 3, 0, 100, 1,
                            1) == PS_OK);
    CHECK(R(0x40020c18u) == 1u << 30);
    puts("PS_TARGET_PASS");
    return 0;
}
