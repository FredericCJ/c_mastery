#ifndef PL_TARGET_H
#define PL_TARGET_H
#include "uart_adapter.h"
/* Dedicated F439 USART3 target; privileged, never execute on host addresses. */
int pl_f439_open(PlUart *, const PuClock *, uint32_t, unsigned, unsigned);
PlTransport pl_f439_transport(PlUart *);
#endif
