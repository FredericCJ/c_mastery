#ifndef CHECKPOINT_TRANSFER_DURABLE_H
#define CHECKPOINT_TRANSFER_DURABLE_H
#include "checkpoint_depth.h"
#include "receipt.h"
typedef struct { store *storage;CPMessage held;unsigned active,durable;int32_t result;int last_error; } CPXSink;
int cpx_sink_accept(CPXSink *,CPQueue *);
int cpx_sink_commit(CPXSink *);
int cpx_sink_acknowledge(CPXSink *,int32_t *);
#endif
