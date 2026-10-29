#include "port.h"
#include "capture.h"
#include "compare.h"
#ifndef G4_BAUD
#define G4_BAUD 115200u
#endif
volatile struct g4_capture g4_capture;
volatile uint32_t g4_release_tx;
static uint32_t ticks;

static uint32_t count_tick(void *unused)
{
    (void)unused;
    return ticks++;
}

void g4_target_main(void)
{
    struct g4_io io = g4_physical_io();
    struct g4_init result = {0};
    int r = g4_initialize(io, G4_BAUD, 100000, &result);
    g4_capture.magic = 0x47345532u;
    g4_capture.version = 2;
    g4_capture.status = (uint32_t)r;
    g4_capture.stage = result.stage;
    g4_capture.polls = result.polls;
    g4_capture.brr = result.brr;
    g4_capture.baud = G4_BAUD;
    if (result.stage >= 3)
    {
        g4_capture.cr1 = io.read(io.context, G4_USART_CR1);
        g4_capture.cr2 = io.read(io.context, G4_USART_CR2);
        g4_capture.cr3 = io.read(io.context, G4_USART_CR3);
        g4_capture.presc = io.read(io.context, G4_USART_PRESC);
        g4_capture.ccipr = io.read(io.context, G4_RCC_CCIPR);
        g4_capture.moder = io.read(io.context, G4_GPIO_MODER);
        g4_capture.afrl = io.read(io.context, G4_GPIO_AFRL);
    }
    g4_capture.part_id = *(
        volatile uint32_t *)0xe0042000u; /* diagnostic only; record revision */
    if (r == G4_OK)
    {
        if (g4_clock_probe(io) != G4_OK)
        {
            g4_capture.status = (uint32_t)G4_CLOCK_TIMEOUT;
            g4_capture.done = 1;
            for (;;)
                __asm__ volatile("nop");
        }
        g4_capture.reserved[2] = io.read(io.context, G4_RCC_CFGR);
        g4_capture.reserved[3] = io.read(io.context, G4_GPIO_AFRH);
        g4_capture.moder = io.read(io.context, G4_GPIO_MODER);
        /* Host instrument barrier, explicitly not a target timebase. After observing
           ready=1, the operator measures PA8 MCO and waits >=1ms before writing the release word. This
           exceeds DS12288's selected 35us stabilization value without pretending
           that an iteration count measures elapsed microseconds. */
        g4_capture.reserved[0] = 1;
        while (g4_release_tx != 0x54585245u)
            __asm__ volatile("nop");
        g4_capture.reserved[1] = 1;
        const uint8_t pattern[] = {0x55, 0x00, 0xff, 0xa5, 0x5a};
        struct pl_byte_port p =
            pl_generated((volatile uint32_t *)0x40004400u, 1, count_tick, 0);
        for (unsigned n = 0; n < sizeof pattern; n++)
        {
            /* W1C TCCF before the next byte avoids crediting an old TC observation. */
            *(volatile uint32_t *)0x40004420u = 1u << 6;
            int tx = pl_byte(p, pattern[n], 100000);
            g4_capture.transfer_status = (uint32_t)tx;
            if (tx)
                break;
            ++g4_capture.bytes;
        }
    }
    g4_capture.count_ticks = ticks;
    if (result.stage >= 3)
        g4_capture.isr = io.read(io.context, G4_USART_ISR);
    __asm__ volatile("dsb" ::: "memory");
    g4_capture.done = 1;
    for (;;)
        __asm__ volatile("nop");
}
