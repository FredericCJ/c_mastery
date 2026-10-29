#ifndef SERIAL_UART_FOLLOWTHROUGH_H
#define SERIAL_UART_FOLLOWTHROUGH_H
#include "uart.h"
#include "serial_depth.h"

struct ua_format
{
    struct sd_uart wire;
    uint32_t cr1, cr2, payload_mask;
};

int ua_format(uint32_t clock, uint32_t baud, unsigned over8, unsigned width,
              unsigned parity, unsigned stop, uint16_t value,
              struct ua_format *);
int ua_apply(struct pu_driver *, struct pu_io, const struct ua_format *);

struct ua_tx
{
    struct pu_driver byte;
    const uint8_t *data;
    unsigned count, offset, chunk, active, stopping, result;
    uint32_t token, deadline, last;
};

int ua_begin(struct ua_tx *, struct pu_io, uint32_t generation, uint32_t token,
             const uint8_t *, unsigned count, uint32_t now, uint32_t budget);
int ua_step(struct ua_tx *, struct pu_io, uint32_t token, uint32_t now);
int ua_cancel(struct ua_tx *, struct pu_io, uint32_t token);

struct ua_rx
{
    struct pu_rx bridge;
    struct pu_byte *slots;
    unsigned capacity, count, head;
};

int ua_rx(struct ua_rx *, struct pu_io);
int ua_take(struct ua_rx *, struct pu_byte *);
#endif
