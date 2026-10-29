#ifndef B06_F439_PORT_H
#define B06_F439_PORT_H
#include "f439_update.h"
/* Exclusive selected F439 HSI16 reset profile. No DMA, IRQ or remapping. */
int f4_cold(struct fh_io);
int f4_uart_init(struct fh_io);
int f4_uart_rx(struct fh_io,unsigned char *,uint32_t *);
int f4_uart_tx(struct fh_io,unsigned char,unsigned);
void f4_fresh(struct fh_io);
struct fm_io f4_target_io(void);
#endif
