#ifndef RS485_BYTE_CHANNEL_V1_H
#define RS485_BYTE_CHANNEL_V1_H
#include "uart_depth.h"
#include "com_edu7.h"

enum { R5_OK,R5_INVALID,R5_BUSY,R5_STALE,R5_FULL,R5_UNAVAILABLE,R5_UNIMPLEMENTED,R5_FRAME };
enum { R5_BYTE=1,R5_FRAMING,R5_OVERRUN,R5_TX_DONE,R5_ABORTED,R5_LOSS,R5_DEVICE_UNAVAILABLE };
enum { R5_CLOSED,R5_IDLE,R5_ENABLING,R5_ACTIVE,R5_TURNAROUND };
enum { R5_THVD1410,R5_THVD1400 };
typedef struct { unsigned device,receiver_disabled;uint32_t tick_ns,guard_ns; } R5Timing;
typedef struct { uint32_t enable,disable,guard; } R5Delays;
int r5_delays(const R5Timing *,R5Delays *);
int r5_load(unsigned eighth_receivers,unsigned bias_eighths,unsigned *total_eighths);
/* Exactly selected callbacks, single foreground owner; they do not reenter,
   allocate or retain borrowed arguments. UART is the actual PuIo producer. */
typedef struct { PuIo uart;void *context;void (*drive)(void *,unsigned); } R5Io;
typedef struct { unsigned kind;uint8_t value,error;uint32_t token; } R5Event;
typedef struct {
    UaTx tx;
    PuRx rx;
    R5Io io;
    R5Delays delays;
    _Alignas(4) uint8_t tx_bytes[16];
    R5Event events[32];
    uint32_t token,last,due,deadline;
    unsigned phase,backend,count,offset,chunk,result,unavailable;
    unsigned head,queued,rx_count,rx_capacity,event_capacity,reserved,echoes;
} R5Channel;
int r5_open(R5Channel *,R5Io,const PuClock *,uint32_t baud,const R5Timing *,
            unsigned rx_capacity,unsigned event_capacity,uint32_t now);
int r5_send(R5Channel *,uint32_t token,const uint8_t *,size_t length,
            unsigned backend,uint32_t now,uint32_t budget,size_t *accepted);
int r5_step(R5Channel *,uint32_t token,uint32_t now);
int r5_cancel(R5Channel *,uint32_t token,uint32_t now);
/* One supplied observation is consumed only on OK; FULL retains caller's event.
   R5_BYTE may carry a raw error mask alongside separate error events. */
int r5_receive(R5Channel *,unsigned status,uint8_t value,size_t *consumed);
int r5_receive_poll(R5Channel *);
int r5_take(R5Channel *,R5Event *,size_t *consumed);
/* Explicit transport adapter notification: unavailable bytes never become clean. */
int r5_loss(R5Channel *);
/* Privileged selected F439 USART3 PD8/PD9 plus dedicated PD10 DE adapter.
   Preparation is exclusive and quiescent; RE is physically tied low. */
void r5_f439_prepare(void);
R5Io r5_f439_io(void);
int r5_f439_send(R5Channel *,uint32_t token,const uint8_t *,size_t length,
                 unsigned backend,uint32_t now,uint32_t budget,size_t *accepted);

typedef struct { EduFramer parser;unsigned local,grant,delivered,rejected,faults; } R5Link;
/* Actual retained EDU7 framer; R5_FRAME alone publishes caller-owned copied
   frame. OK consumes a rejected/fault/control event; BUSY retains a prefix. */
int r5_link(R5Link *,const R5Event *,EduFrame *);
int r5_test(unsigned);
int r5_mapped(void);
#endif
