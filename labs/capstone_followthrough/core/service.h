#ifndef CAPSTONE_SERVICE_H
#define CAPSTONE_SERVICE_H
#include <stddef.h>
#include <stdint.h>
#include "checkpoint_depth.h"
enum { CS_SESSIONS=2, CS_QUEUE=3, CS_WIRE=42, CS_MAX_ADMISSIONS=65535 };
typedef enum { CS_OK,CS_MORE,CS_FRAME,CS_REJECT,CS_FULL,CS_DENIED,CS_CLOSED,
               CS_PENDING,CS_DONE,CS_STALE,CS_INVALID,CS_FAILED } CsStatus;
typedef enum { CS_OPEN,CS_DRAIN,CS_CANCEL,CS_STOPPED } CsState;
typedef enum { CS_IO_PROGRESS,CS_IO_AGAIN,CS_IO_INTERRUPTED,CS_IO_ERROR } CsIOKind;
typedef struct { size_t accepted; CsIOKind kind; unsigned cause; } CsIO;
typedef CsIO (*CsSink)(void *,const unsigned char *,size_t);
typedef void (*CsDone)(void *,uint16_t,CsStatus);
typedef struct { unsigned char bytes[CS_WIRE]; size_t length; uint16_t sequence; int replaceable; } CsWire;
typedef struct { unsigned char bytes[CS_WIRE]; size_t used; } CsInput;
typedef struct {
    CsInput input[CS_SESSIONS]; CsWire queue[CS_QUEUE],active;
    size_t head,count,capacity,offset;
    unsigned generation,limit,accepted,delivered,discarded,releases,rejected,denied;
    unsigned first_cause; int has_active,borrowed,failed; CsState state;
} CsService;
CsStatus cs_init(CsService *,size_t,unsigned);
CsStatus cs_offer(CsService *,const CPMessage *,int authorized,int replaceable,int overwrite);
CsStatus cs_feed(CsService *,unsigned,const unsigned char *,size_t,size_t *,int authorized,int replaceable);
CsStatus cs_begin(CsService *);
CsStatus cs_step(CsService *,CsSink,void *);
CsStatus cs_recover(CsService *);
CsStatus cs_lease(CsService *,const unsigned char **,size_t *,unsigned *);
CsStatus cs_complete(CsService *,unsigned,int quiescent,size_t,CsDone,void *);
CsStatus cs_close(CsService *,int cancel);
int cs_conserved(const CsService *);
/* No POSIX/register/allocator type occurs in this portable interface. */
#endif
