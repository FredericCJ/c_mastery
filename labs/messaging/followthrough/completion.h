#ifndef CMASTERY_MESSAGING_COMPLETION_H
#define CMASTERY_MESSAGING_COMPLETION_H
#include "operation.h"

/* Independent control lifetime: a raw pointer is not a retained token. */
typedef struct {
    atomic_size_t references;
    atomic_uint flags;
    MsgOperation operation;
    MfoReceiver receiver;
    void *context;
    MfoValue pending;
    void (*released)(void *);
    void *released_context;
} MfxCompletion;

MsgStatus mfx_create(MfxCompletion **, MfoReceiver, void *, void (*)(void *), void *, bool);
MsgStatus mfx_retain(MfxCompletion *, MfxCompletion **);
void mfx_drop(MfxCompletion **);
MsgStatus mfx_cancel(MfxCompletion *);
MsgStatus mfx_complete(MfxCompletion *, MsgStatus, MfoValue *);
#endif
