#ifndef CMASTERY_MESSAGING_OPERATION_FOLLOW_H
#define CMASTERY_MESSAGING_OPERATION_FOLLOW_H
#include "messaging.h"
#include "owned_value.h"
struct mfo_operation;
typedef void (*mfo_receiver_fn)(void *, enum msg_status, struct mfo_value);
typedef void (*mfo_backend_fn)(struct mfo_operation *, void *);

struct mfo_operation
{
    struct msg_operation operation;
    mfo_receiver_fn receiver;
    void *receiver_ctx;
    mfo_backend_fn backend;
    void *backend_ctx;
    struct mfo_value pending;
};
enum msg_status mfo_init(struct mfo_operation *, mfo_backend_fn, void *,
                         mfo_receiver_fn, void *);
enum msg_status mfo_start(struct mfo_operation *);
enum msg_status mfo_stop(struct mfo_operation *);
/* On accepted completion, consumes *value even for failure/cancellation.
 * Receiver owns it and may destroy this operation before this function returns. */
enum msg_status mfo_finish(struct mfo_operation *, enum msg_status,
                           struct mfo_value *);
#endif
