#include "operation.h"
#ifndef MFO_MUTANT
#define MFO_MUTANT 0
#endif
static void delivered(void *arg, MsgStatus status, unsigned ignored) {
  MfoOperation *op = arg;
  MfoReceiver receiver = op->receiver;
  void *ctx = op->receiver_ctx;
  MfoValue value = op->pending;
  (void)ignored;
  op->pending = (MfoValue){0};
  if (MFO_MUTANT == 2) value = (MfoValue){0};
  receiver(ctx, status, value);
  /* Receiver may have freed op, ctx and the backend context. */
}
MsgStatus mfo_init(MfoOperation *op, MfoBackend backend, void *backend_ctx,
                   MfoReceiver receiver, void *receiver_ctx) {
  if (!op || !backend || !receiver) return MSG_INVALID;
  *op = (MfoOperation){.receiver=receiver,.receiver_ctx=receiver_ctx,
                       .backend=backend,.backend_ctx=backend_ctx};
  msg_connect(&op->operation,delivered,op);
  return MSG_OK;
}
MsgStatus mfo_start(MfoOperation *op) {
  if (!op) return MSG_INVALID;
  MfoBackend backend = op->backend;
  void *ctx = op->backend_ctx;
  MsgStatus status = MFO_MUTANT == 1 ? MSG_OK : msg_start(&op->operation);
  if (status != MSG_OK) return status;
  backend(op,ctx);
  return MSG_OK;
}
MsgStatus mfo_stop(MfoOperation *op) {
  return op ? msg_stop_operation(&op->operation) : MSG_INVALID;
}
MsgStatus mfo_finish(MfoOperation *op, MsgStatus status, MfoValue *value) {
  if (!op || !value || (value->data && !value->drop) ||
      (status != MSG_OK && status != MSG_FAILED && status != MSG_CANCELLED))
    return MSG_INVALID;
  if (op->operation.state != MSG_STARTED) return MSG_BUSY;
  op->pending = *value;
  *value = (MfoValue){0};
  return msg_complete(&op->operation,status,0);
}
