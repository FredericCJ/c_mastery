#ifndef CHECKPOINT_TRANSFER_DURABLE_H
#define CHECKPOINT_TRANSFER_DURABLE_H
#include "checkpoint_depth.h"
#include "receipt.h"
struct cpx_sink { store *storage;struct cp_message held;unsigned active,durable;int32_t result;int last_error; };
int cpx_sink_accept(struct cpx_sink *,struct cp_queue *);
int cpx_sink_commit(struct cpx_sink *);
int cpx_sink_acknowledge(struct cpx_sink *,int32_t *);
#endif
