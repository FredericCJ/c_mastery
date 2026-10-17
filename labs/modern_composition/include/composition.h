#ifndef CMASTERY_MODERN_COMPOSITION_H
#define CMASTERY_MODERN_COMPOSITION_H
#include "errors.h"
#include "ownership.h"
#include "coroutines.h"
#include "messaging.h"

/* Separate parse and ownership failures instead of inventing a lossy common
   error number. A valid owned published object and exclusive access are required. */
typedef struct { er_result parse; OwStatus storage; bool committed; } McPublish;
McPublish mc_publish_pair(const unsigned char *, size_t, V4Allocator, OwOwner *);

/* Stable storage from successful initialization through terminal notification.
   The port is serialized, does not reenter, and disarm quiesces queued callbacks.
   The receiver may destroy the enclosing McRead. No access is permitted after it. */
typedef struct {
    COAsync frame;
    COAsyncPort port;
    MsgOperation operation;
    OwScope cleanup;
    unsigned char *output;
    bool terminal;
} McRead;
MsgStatus mc_read_init(McRead *, COAsyncPort, size_t, uint32_t,
                       unsigned char[8], MsgReceiver, void *, OwCleanup, void *);
MsgStatus mc_read_poll(McRead *);
MsgStatus mc_read_cancel(McRead *);
#endif
