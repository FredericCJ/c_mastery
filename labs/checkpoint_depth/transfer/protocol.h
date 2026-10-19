#ifndef CHECKPOINT_TRANSFER_PROTOCOL_H
#define CHECKPOINT_TRANSFER_PROTOCOL_H
#include <stddef.h>
#include <stdint.h>
#ifndef CPX_MAX_PAYLOAD
#define CPX_MAX_PAYLOAD 64
#endif
enum { CPX_WIRE_MAX=CPX_MAX_PAYLOAD+15 };
typedef enum { CPX_P_OK,CPX_P_MORE,CPX_P_FRAME,CPX_P_REJECT,CPX_P_FULL,CPX_P_INVALID,CPX_P_NOT_IMPLEMENTED } CPXProtocolStatus;
typedef struct { unsigned version,flags;uint16_t sequence;uint32_t stamp;size_t length;unsigned char payload[CPX_MAX_PAYLOAD]; } CPXMessage;
typedef struct { unsigned char candidate[CPX_WIRE_MAX];size_t used,limit;unsigned ready;CPXMessage message; } CPXParser;
typedef struct { CPXMessage items[4];size_t capacity,head,count; } CPXQueue;
CPXProtocolStatus cpx_protocol_init(CPXParser *,size_t);
CPXProtocolStatus cpx_protocol_feed(CPXParser *,const unsigned char *,size_t,size_t *);
CPXProtocolStatus cpx_protocol_queue(CPXQueue *,size_t);
CPXProtocolStatus cpx_protocol_deliver(CPXParser *,CPXQueue *);
CPXProtocolStatus cpx_protocol_take(CPXQueue *,CPXMessage *);
CPXProtocolStatus cpx_protocol_encode(const CPXMessage *,unsigned char *,size_t,size_t *);
CPXProtocolStatus cpx_protocol_inplace(unsigned char *,size_t,size_t,unsigned,unsigned,uint16_t,uint32_t,size_t *);
#endif
