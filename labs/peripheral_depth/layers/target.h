#ifndef PL_TARGET_H
#define PL_TARGET_H
#include "uart_adapter.h"
/* Dedicated F439 USART3 target; privileged, never execute on host addresses. */
int pl_f439_open(struct pl_uart *, const struct pu_clock *, uint32_t, unsigned, unsigned);
struct pl_transport pl_f439_transport(struct pl_uart *);
#endif
