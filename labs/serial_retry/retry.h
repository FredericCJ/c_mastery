#ifndef SERIAL_RETRY_H
#define SERIAL_RETRY_H
#include "systems.h"
#include "serial_depth.h"
#include "timing.h"
#include "i2c.h"
enum { SR_OK, SR_PENDING, SR_READY, SR_DONE, SR_INVALID, SR_AMBIGUOUS, SR_TIMEOUT, SR_LOST, SR_NACK, SR_UNCERTAIN, SR_EXHAUSTED, SR_STALE, SR_UNIMPLEMENTED };
typedef struct { unsigned bits; uint32_t last,remaining; } SrTimer;
int sr_timer_begin(SrTimer *,unsigned,uint32_t,uint32_t);
int sr_timer_step(SrTimer *,uint32_t,uint32_t *);
typedef struct { SrTimer timer; SI2c bit; uint32_t stretch,low_ticks; unsigned low,terminal; } SrByte;
int sr_byte_begin(SrByte *,unsigned,unsigned,uint32_t,uint32_t,uint32_t);
int sr_byte_sample(SrByte *,uint32_t,unsigned,unsigned,unsigned);
typedef struct { SrTimer timer; SDI2c bus; SI2c bit; uint32_t token; uint8_t address,wire; unsigned action,terminal; } SrTxn;
int sr_txn_begin(SrTxn *,uint32_t,uint8_t,uint8_t,unsigned,uint32_t,uint32_t);
int sr_txn_bit(SrTxn *,uint32_t,uint32_t,unsigned,unsigned);
int sr_txn_read(SrTxn *,uint32_t,uint32_t,uint8_t,uint8_t *);
typedef struct { SrTimer timer; uint32_t token,free_ticks,wait_ticks,bus_free,backoff; unsigned command,maximum,attempts,state,free; } SrRetry;
int sr_retry_begin(SrRetry *,unsigned,uint32_t,uint32_t,uint32_t,unsigned,unsigned,uint32_t,uint32_t);
int sr_retry_poll(SrRetry *,uint32_t,unsigned,unsigned,unsigned);
int sr_retry_complete(SrRetry *,uint32_t,uint32_t,unsigned,unsigned);
#endif
