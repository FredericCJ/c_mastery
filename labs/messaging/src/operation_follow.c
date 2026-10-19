#include "operation.h"
MsgStatus mfo_init(MfoOperation *op,MfoBackend backend,void *bc,MfoReceiver receiver,void *rc) {
  (void)op;(void)backend;(void)bc;(void)receiver;(void)rc;return MSG_FAILED;
}
MsgStatus mfo_start(MfoOperation *op) {(void)op;return MSG_FAILED;}
MsgStatus mfo_stop(MfoOperation *op) {(void)op;return MSG_FAILED;}
MsgStatus mfo_finish(MfoOperation *op,MsgStatus s,MfoValue *v) {(void)op;(void)s;(void)v;return MSG_FAILED;}
