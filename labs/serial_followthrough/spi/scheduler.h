#ifndef SERIAL_SPI_SCHEDULER_H
#define SERIAL_SPI_SCHEDULER_H
#include "spi.h"
#include "algo.h"
typedef struct {
 uint32_t token;unsigned state,count,dma,budget,setup,hold,result;
 uint8_t tx[16],rx[16];uint16_t tx_words[16],rx_words[16];
} SsSlot;
typedef struct {PsSpi driver;algo_ring pending;SsSlot slots[2];uint32_t last_token;unsigned initialized,active;} SsScheduler;
typedef struct {unsigned status,sent,received;} SsResult;
typedef int (*SsSubmit)(PsSpi *,PsIo,uint32_t,uint32_t,const uint8_t *,uint8_t *,const uint16_t *,uint16_t *,unsigned,unsigned,uint32_t,unsigned,unsigned,unsigned);
enum {SS_FREE,SS_QUEUED,SS_RUNNING,SS_DONE};
int ss_open(SsScheduler *,PsIo,const PuClock *,unsigned mode,unsigned divider);
int ss_enqueue(SsScheduler *,uint32_t token,const uint8_t *,unsigned count,unsigned dma,unsigned budget,unsigned setup,unsigned hold);
int ss_run(SsScheduler *,PsIo,SsSubmit,uint32_t now);
int ss_event(SsScheduler *,PsIo,uint32_t token,uint32_t now);
int ss_cancel(SsScheduler *,PsIo,uint32_t token);
int ss_reset(SsScheduler *,PsIo,uint32_t token,uint32_t now);
int ss_take(SsScheduler *,uint32_t token,SsResult *,uint8_t output[16]);
int ss_portable_submit(PsSpi *,PsIo,uint32_t,uint32_t,const uint8_t *,uint8_t *,const uint16_t *,uint16_t *,unsigned,unsigned,uint32_t,unsigned,unsigned,unsigned);
int ss_f439_submit(PsSpi *,PsIo,uint32_t,uint32_t,const uint8_t *,uint8_t *,const uint16_t *,uint16_t *,unsigned,unsigned,uint32_t,unsigned,unsigned,unsigned);
#endif
