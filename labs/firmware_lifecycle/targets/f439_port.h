#ifndef B06_F439_PORT_H
#define B06_F439_PORT_H
#include "f439_update.h"
/* Exclusive selected F439 HSI16 reset profile. No DMA, IRQ or remapping. */
int f4_cold(FHIO);
int f4_uart_init(FHIO);
int f4_uart_rx(FHIO,unsigned char *,uint32_t *);
int f4_uart_tx(FHIO,unsigned char,unsigned);
void f4_fresh(FHIO);
FMIO f4_target_io(void);
#endif
