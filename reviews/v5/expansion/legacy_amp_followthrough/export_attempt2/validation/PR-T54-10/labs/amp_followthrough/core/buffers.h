#ifndef AF_BUFFERS_H
#define AF_BUFFERS_H
#include "amp.h"
#include "bridge.h"
enum { AF_FREE,AF_FILLING,AF_DEVICE,AF_COMPLETED,AF_CONSUMING };
typedef struct { XdLease lease;XdTicket ticket;XdResult result;unsigned phase; } AfBuffer;
typedef struct { PdTransfer transfer;AfBuffer slot[2];unsigned active; } AfBuffers;
/* Serialized owner of one hardware stream and two disjoint whole-line buffers.
 * Initialization requires a trusted cold or externally fenced device. */
int af_buffers_init(AfBuffers *,const XdFence *);
int af_fill(AfBuffers *,unsigned);
int af_submit(AfBuffers *,PdIo,unsigned,const PdExtent *);
int af_timeout(AfBuffers *,PdIo);
int af_complete(AfBuffers *,PdIo);
int af_consume(AfBuffers *,unsigned);
int af_release(AfBuffers *,unsigned);
#endif
