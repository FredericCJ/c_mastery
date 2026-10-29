#include "operation.h"
enum msg_status mfo_init(struct mfo_operation *op,mfo_backend_fn backend,void *bc,mfo_receiver_fn receiver,void *rc) {
  (void)op;(void)backend;(void)bc;(void)receiver;(void)rc;return MSG_FAILED;
}
enum msg_status mfo_start(struct mfo_operation *op) {(void)op;return MSG_FAILED;}
enum msg_status mfo_stop(struct mfo_operation *op) {(void)op;return MSG_FAILED;}
enum msg_status mfo_finish(struct mfo_operation *op,enum msg_status s,struct mfo_value *v) {(void)op;(void)s;(void)v;return MSG_FAILED;}
