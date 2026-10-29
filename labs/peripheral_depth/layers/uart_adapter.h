#ifndef PERIPHERAL_LAYERS_UART_H
#define PERIPHERAL_LAYERS_UART_H
#include "service.h"
#include "uart.h"

struct pl_uart
{
    struct pu_driver driver;
    struct pu_io io;
    uint32_t generation;
    unsigned backend;
};

int pl_uart_open(struct pl_uart *, struct pu_io, const struct pu_clock *,
                 uint32_t, unsigned, unsigned);
struct pl_transport pl_uart_transport(struct pl_uart *);
#endif
