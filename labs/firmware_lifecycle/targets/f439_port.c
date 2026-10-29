#include "f439_port.h"
int f4_cold(struct fh_io x){(void)x;return 0;}
int f4_uart_init(struct fh_io x){(void)x;return 0;}
int f4_uart_rx(struct fh_io x,unsigned char*b,uint32_t*s){(void)x;(void)b;(void)s;return -1;}
int f4_uart_tx(struct fh_io x,unsigned char b,unsigned n){(void)x;(void)b;(void)n;return 0;}
void f4_fresh(struct fh_io x){(void)x;}
