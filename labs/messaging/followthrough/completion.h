#ifndef CMASTERY_MESSAGING_COMPLETION_H
#define CMASTERY_MESSAGING_COMPLETION_H
#include "operation.h"

/* Independent control lifetime: a raw pointer is not a retained token. */
struct mfx_completion
{
    atomic_size_t references;
    atomic_uint flags;
    struct msg_operation operation;
    mfo_receiver_fn receiver;
    void *context;
    struct mfo_value pending;
    void (*released)(void *);
    void *released_context;
};

enum msg_status mfx_create(struct mfx_completion **, mfo_receiver_fn, void *,
                           void (*)(void *), void *, bool);
enum msg_status mfx_retain(struct mfx_completion *, struct mfx_completion **);
void mfx_drop(struct mfx_completion **);
enum msg_status mfx_cancel(struct mfx_completion *);
enum msg_status mfx_complete(struct mfx_completion *, enum msg_status,
                             struct mfo_value *);
#endif
