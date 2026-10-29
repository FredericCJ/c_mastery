#include "operation.h"
#ifndef MFO_MUTANT
#define MFO_MUTANT 0
#endif
static void delivered(void *arg, enum msg_status status, unsigned ignored) {
  struct mfo_operation *op = arg;
  mfo_receiver_fn receiver = op->receiver;
  void *ctx = op->receiver_ctx;
  struct mfo_value value = op->pending;
  (void)ignored;
  op->pending = (struct mfo_value){0};
  if (MFO_MUTANT == 2) value = (struct mfo_value){0};
  receiver(ctx, status, value);
  /* Receiver may have freed op, ctx and the backend context. */
}
enum msg_status mfo_init(struct mfo_operation *op, mfo_backend_fn backend, void *backend_ctx,
                   mfo_receiver_fn receiver, void *receiver_ctx) {
  if (!op || !backend || !receiver) return MSG_INVALID;
  *op = (struct mfo_operation){.receiver=receiver,.receiver_ctx=receiver_ctx,
                       .backend=backend,.backend_ctx=backend_ctx};
  msg_connect(&op->operation,delivered,op);
  return MSG_OK;
}
enum msg_status mfo_start(struct mfo_operation *op) {
  if (!op) return MSG_INVALID;
  mfo_backend_fn backend = op->backend;
  void *ctx = op->backend_ctx;
  enum msg_status status = MFO_MUTANT == 1 ? MSG_OK : msg_start(&op->operation);
  if (status != MSG_OK) return status;
  backend(op,ctx);
  return MSG_OK;
}
enum msg_status mfo_stop(struct mfo_operation *op) {
  return op ? msg_stop_operation(&op->operation) : MSG_INVALID;
}
enum msg_status mfo_finish(struct mfo_operation *op, enum msg_status status, struct mfo_value *value) {
  if (!op || !value || (value->data && !value->drop) ||
      (status != MSG_OK && status != MSG_FAILED && status != MSG_CANCELLED))
    return MSG_INVALID;
  if (op->operation.state != MSG_STARTED) return MSG_BUSY;
  op->pending = *value;
  *value = (struct mfo_value){0};
  return msg_complete(&op->operation,status,0);
}
