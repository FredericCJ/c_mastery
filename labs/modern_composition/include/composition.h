#ifndef CMASTERY_MODERN_COMPOSITION_H
#define CMASTERY_MODERN_COMPOSITION_H
#include "errors.h"
#include "ownership.h"
#include "coroutines.h"
#include "messaging.h"

/* Separate parse and ownership failures instead of inventing a lossy common
   error number. A valid owned published object and exclusive access are required. */
struct mc_publish
{
    er_result parse;
    enum ow_status storage;
    bool committed;
};
struct mc_publish mc_publish_pair(const unsigned char *, size_t,
                                  struct v4_allocator, struct ow_owner *);

/* Stable storage from successful initialization through terminal notification.
   The port is serialized, does not reenter, and disarm quiesces queued callbacks.
   The receiver may destroy the enclosing McRead. No access is permitted after it. */
struct mc_read
{
    struct co_async frame;
    struct co_async_port port;
    struct msg_operation operation;
    struct ow_scope cleanup;
    unsigned char *output;
    bool terminal;
};
enum msg_status mc_read_init(struct mc_read *, struct co_async_port, size_t,
                             uint32_t, unsigned char[8], msg_receiver_fn,
                             void *, ow_cleanup_fn, void *);
enum msg_status mc_read_poll(struct mc_read *);
enum msg_status mc_read_cancel(struct mc_read *);
#endif
