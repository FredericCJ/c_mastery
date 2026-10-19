#ifndef CMASTERY_MESSAGING_OPERATION_FOLLOW_H
#define CMASTERY_MESSAGING_OPERATION_FOLLOW_H
#include "messaging.h"
#include "owned_value.h"
typedef struct MfoOperation MfoOperation;
typedef void (*MfoReceiver)(void *, MsgStatus, MfoValue);
typedef void (*MfoBackend)(MfoOperation *, void *);
struct MfoOperation {
  MsgOperation operation;
  MfoReceiver receiver;
  void *receiver_ctx;
  MfoBackend backend;
  void *backend_ctx;
  MfoValue pending;
};
MsgStatus mfo_init(MfoOperation *, MfoBackend, void *, MfoReceiver, void *);
MsgStatus mfo_start(MfoOperation *);
MsgStatus mfo_stop(MfoOperation *);
/* On accepted completion, consumes *value even for failure/cancellation.
 * Receiver owns it and may destroy this operation before this function returns. */
MsgStatus mfo_finish(MfoOperation *, MsgStatus, MfoValue *);
#endif
