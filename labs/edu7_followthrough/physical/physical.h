#ifndef EDU7_PHYSICAL_V1_H
#define EDU7_PHYSICAL_V1_H
#include "channel.h"
#include "edu7_depth.h"
#include "foundation.h"
enum { EP_OK,EP_INVALID,EP_BUSY,EP_STALE,EP_FULL,EP_TIMEOUT,EP_FRAME,EP_DROP,EP_STUB=99 };
enum { EP_IDLE,EP_ENABLE,EP_ACTIVE,EP_GUARD };
typedef struct {
 PuDriver byte;R5Io io;R5Delays delays;
 uint8_t bytes[96];uint32_t token,last,due,deadline,clock,divisor,first_deadline;
 unsigned phase,count,offset,chunk,result,cancel,enabled;
} EpTx;
typedef struct {
 EpTx tx;E7Grant grant;EfFramer parser;
 uint32_t token,last,echoes,faults,drops,delivered;
 unsigned polling;
} EpMaster;
typedef struct {EpTx tx;EfFramer parser;uint32_t last,origin;unsigned granted;} EpSlave;
/* Fresh zeroed owner; fixed microsecond clock, nonwrapping campaign time. */
int ep_open(EpTx *,R5Io,const PuClock *,uint32_t,const R5Timing *,uint32_t);
int ep_fits(const EpTx *,unsigned,uint32_t,unsigned *);
int ep_send(EpTx *,uint32_t,const uint8_t *,size_t,uint32_t,uint32_t);
int ep_step(EpTx *,uint32_t,uint32_t);
int ep_cancel(EpTx *,uint32_t,uint32_t);
int ep_poll(EpMaster *,uint32_t,uint32_t);
int ep_tick(EpMaster *,uint32_t);
int ep_receive(EpMaster *,unsigned,uint8_t,uint32_t,EduFrame *);
int ep_master_poll_rx(EpMaster *,uint32_t,EduFrame *);
int ep_disconnect(EpMaster *,uint32_t);
int ep_slave_receive(EpSlave *,unsigned,uint8_t,uint32_t);
int ep_slave_poll_rx(EpSlave *,uint32_t);
int ep_reply(EpSlave *,uint32_t,const EduFrame *,uint32_t);
int ep_f439_open(EpTx *,const PuClock *,uint32_t,unsigned,uint32_t);
int ep_f439_open_port(EpTx *,const PuClock *,uint32_t,unsigned,unsigned,uint32_t);
int ep_mapped(void);
int ep_tests(unsigned);
#endif
