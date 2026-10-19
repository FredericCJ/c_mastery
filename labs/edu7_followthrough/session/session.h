#ifndef EDU7_SESSION_SUCCESSOR_H
#define EDU7_SESSION_SUCCESSOR_H
#include "transport.h"
enum {ES_CLOSED,ES_OPENING,ES_ACK_NEEDED,ES_ACKING,ES_READY};
enum {ES_DELIVERED=30,ES_DUPLICATE=31,ES_STUB=99};
typedef struct {
 E7Session core;EtTx tx;EtRx rx;
 unsigned self,peer,phase,uncertain,outgoing_kind,effects,responses;
 uint16_t sent_checkpoint,outgoing_checkpoint,outgoing_id,last_control,error_session,error_epoch;
 uint8_t sent[80],sent_length;
 int16_t setpoint;EduRecord response;
} EsPeer;
typedef struct {unsigned type,duplicate;uint16_t checkpoint;uint8_t reply[80];size_t length;} EsEvent;
int es_init(EsPeer *,unsigned,unsigned,uint16_t);
int es_emit(EsPeer *,unsigned,uint16_t,uint16_t,uint16_t,const uint8_t *,size_t,uint16_t,uint32_t);
int es_offer(EsPeer *,uint32_t,uint8_t *,size_t,size_t *,uint32_t *);
int es_admit(EsPeer *,uint16_t,uint32_t,unsigned,uint32_t);
int es_ack(EsPeer *,const uint8_t *,size_t,uint32_t);
int es_receive(EsPeer *,const uint8_t *,size_t,uint32_t,uint8_t *,size_t,size_t *);
int es_deliver(EsPeer *,size_t,EsEvent *);
int es_uncertain(EsPeer *,uint16_t,uint16_t);
int es_permitted(const EsPeer *);
int es_tests(unsigned);
#endif
