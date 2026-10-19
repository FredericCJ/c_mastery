#ifndef PERIPHERAL_LAYERS_UART_H
#define PERIPHERAL_LAYERS_UART_H
#include "service.h"
#include "uart.h"
typedef struct {
  PuDriver driver;
  PuIo io;
  uint32_t generation;
  unsigned backend;
} PlUart;
int pl_uart_open(PlUart *, PuIo, const PuClock *, uint32_t, unsigned, unsigned);
PlTransport pl_uart_transport(PlUart *);
#endif
