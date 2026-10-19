#ifndef SERIAL_UART_FOLLOWTHROUGH_H
#define SERIAL_UART_FOLLOWTHROUGH_H
#include "uart.h"
#include "serial_depth.h"
typedef struct { SDUart wire; uint32_t cr1,cr2,payload_mask; } UaFormat;
int ua_format(uint32_t clock,uint32_t baud,unsigned over8,unsigned width,
              unsigned parity,unsigned stop,uint16_t value,UaFormat *);
int ua_apply(PuDriver *,PuIo,const UaFormat *);
typedef struct {
  PuDriver byte;
  const uint8_t *data;
  unsigned count,offset,chunk,active,stopping,result;
  uint32_t token,deadline,last;
} UaTx;
int ua_begin(UaTx *,PuIo,uint32_t generation,uint32_t token,
             const uint8_t *,unsigned count,uint32_t now,uint32_t budget);
int ua_step(UaTx *,PuIo,uint32_t token,uint32_t now);
int ua_cancel(UaTx *,PuIo,uint32_t token);
typedef struct { PuRx bridge; PuByte *slots;unsigned capacity,count,head; } UaRx;
int ua_rx(UaRx *,PuIo);
int ua_take(UaRx *,PuByte *);
#endif
