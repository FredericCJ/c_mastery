#ifndef CMASTERY_EPT_H
#define CMASTERY_EPT_H
#include "errors.h"
#include "stream.h"
enum {EPT_OK,EPT_INVALID,EPT_RANGE,EPT_BUSY,EPT_AGAIN,EPT_IO,EPT_CANCELLED,
      EPT_UNCERTAIN,EPT_DONE,EPT_STALE,EPT_CONTRACT,EPT_UNIMPLEMENTED};
enum {EPT_FLUSH_OK,EPT_FLUSH_IO,EPT_FLUSH_UNKNOWN};
typedef struct {size_t prefix;uint32_t epoch;unsigned code;} EptFlushResult;
typedef EptFlushResult (*EptFlush)(void *,size_t,size_t,uint32_t);
typedef struct {
    SFStream frame;SFHandle base;size_t durable;uint32_t epoch;
    unsigned active,state,uncertain;
} EptStream;
typedef struct {EptStream *owner;uint32_t generation;} EptHandle;
typedef struct {size_t accepted,durable;uint32_t epoch;unsigned state,uncertain;} EptObservation;
/* Fresh storage only; live, noncopied control remains at the same address.
   All calls externally serialized; callbacks synchronous and nonreentrant. */
void ept_init(EptStream *);
unsigned ept_start(EptStream *,const unsigned char *,size_t,EptHandle *);
unsigned ept_step(EptHandle,er_sink,void *,EptObservation *);
unsigned ept_cancel(EptHandle,EptObservation *);
unsigned ept_checkpoint(EptHandle,EptFlush,void *,EptObservation *);
/* Trusted backend authority after known flush, not a proof from readable bytes. */
unsigned ept_reconcile(EptHandle,size_t,uint32_t,unsigned,EptObservation *);
unsigned ept_observe(EptHandle,EptObservation *);
unsigned ept_release(EptHandle,unsigned,unsigned);
#endif
