#ifndef EDU7_TRANSPORT_SUCCESSOR_H
#define EDU7_TRANSPORT_SUCCESSOR_H
#include "com_edu7.h"
#include "edu7_depth.h"
enum { ET_COMPLETE=20,ET_STUB=99 };
typedef struct {uint16_t epoch;uint8_t profile,node,peer,port,peer_port;} EtIdentity;
typedef struct {EtIdentity identity;E7Sender inner;unsigned peer_credit;} EtTx;
typedef struct {EtIdentity identity;EduRx inner;uint16_t retired;uint32_t last,started,expired;} EtRx;
int et_tx_open(EtTx *,unsigned,uint16_t,unsigned);
int et_rx_open(EtRx *,unsigned,uint16_t,unsigned);
int et_begin(EtTx *,uint16_t,const uint8_t *,size_t,uint32_t);
int et_offer(EtTx *,uint32_t,uint8_t *,size_t,size_t *,uint32_t *);
int et_accept(EtTx *,uint16_t,uint32_t,unsigned,uint32_t);
int et_ack(EtTx *,const uint8_t *,size_t,uint32_t);
int et_receive(EtRx *,const uint8_t *,size_t,uint32_t,uint8_t *,size_t,size_t *);
int et_expire(EtRx *,uint32_t);
int et_peek(const EtRx *,uint8_t *,size_t,size_t *);
int et_release(EtRx *,uint16_t,uint16_t);
int et_tests(unsigned);
#endif
