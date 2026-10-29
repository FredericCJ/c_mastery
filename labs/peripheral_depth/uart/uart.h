#ifndef PERIPHERAL_UART_DEPTH_H
#define PERIPHERAL_UART_DEPTH_H
#include <stddef.h>
#include <stdint.h>

enum
{
    PU_OK,
    PU_INVALID,
    PU_BUSY,
    PU_TIMEOUT,
    PU_STALE,
    PU_FULL,
    PU_IO,
    PU_CANCELLED,
    PU_UNIMPLEMENTED
};

enum
{
    PU_POLL,
    PU_IRQ,
    PU_DMA
};

enum
{
    PU_SOURCE,
    PU_GATE,
    PU_RESET,
    PU_SR,
    PU_DR,
    PU_BRR,
    PU_CR1,
    PU_CR2,
    PU_CR3,
    PU_DMA_GATE,
    PU_DMA_BUSY,
    PU_DMA_ADDRESS,
    PU_DMA_COUNT,
    PU_DMA_FLAGS,
    PU_DMA_CLEAR,
    PU_DMA_CONTROL
};

enum
{
    PU_PE = 1,
    PU_FE = 2,
    PU_NF = 4,
    PU_ORE = 8,
    PU_RXNE = 32,
    PU_TC = 64,
    PU_TXE = 128
};

struct pu_io
{
    void *context;
    uint32_t (*read)(void *, unsigned);
    void (*write)(void *, unsigned, uintptr_t);
};

struct pu_clock
{
    uint32_t sys_hz, generation;
    unsigned ahb, apb1, apb2, timpre;
};

struct pu_rates
{
    uint32_t core, apb1, apb2, timer1, timer2;
};

int pu_rates(const struct pu_clock *, struct pu_rates *);

struct pu_byte
{
    uint8_t value, error;
};

struct pu_rx
{
    struct pu_byte items[2];
    unsigned head, count, backpressure, reserve;
    uint32_t dropped, errors;
};

int pu_receive(struct pu_rx *, unsigned status, uint8_t value);
int pu_take(struct pu_rx *, struct pu_byte *);
int pu_peer_ready(const struct pu_rx *, unsigned *);

struct pu_driver
{
    uint32_t configured_generation, token, last, deadline;
    unsigned ready, active, backend, count, position, memory_owned, line_owned,
        cancel, result;
    const uint8_t *buffer;
};

int pu_open(struct pu_driver *, struct pu_io, const struct pu_clock *,
            uint32_t baud, unsigned over8, unsigned polls);
int pu_submit(struct pu_driver *, struct pu_io, uint32_t generation,
              uint32_t token, unsigned backend, const uint8_t *, unsigned count,
              uint32_t now, uint32_t budget);
int pu_step(struct pu_driver *, struct pu_io, uint32_t token, uint32_t now);
int pu_cancel(struct pu_driver *, struct pu_io, uint32_t token);
int pu_reset_notice(struct pu_driver *, struct pu_io);
int pu_poll(struct pu_driver *, struct pu_io, uint32_t token, uint32_t now,
            unsigned polls);
int pu_rx_poll(struct pu_rx *, struct pu_io, unsigned polls);
int pu_deadline_fit(uint32_t clock, uint32_t divisor, unsigned bytes,
                    uint32_t remaining_us, unsigned *fits);
/* Privileged F439-only adapter; never call on a hosted machine. */
struct pu_io pu_f439_io(void);
void pu_f439_pins(void);
void pu_f439_rts_init(void);
int pu_f439_rts(const struct pu_rx *);
int pu_f439_dma_submit(struct pu_driver *, uint32_t generation, uint32_t token,
                       const uint8_t *, unsigned count, uint32_t now,
                       uint32_t budget);
#endif
