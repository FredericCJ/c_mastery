#include "composition.h"
#include <limits.h>
#ifndef MC_MUTANT
#define MC_MUTANT 0
#endif

struct mc_publish mc_publish_pair(const unsigned char *bytes, size_t n,
                                  struct v4_allocator a,
                                  struct ow_owner *published)
{
    struct mc_publish r = {er_error(ER_INVALID, 0), OW_OK, false};
    if (!published || n > INT32_MAX)
        return r;
    uint32_t sum = 0;
    r.parse = er_parse_pair(bytes, n, &sum);
    if (r.parse.tag != ER_OK)
        return r;
    struct ow_owner staged = {0};
    const size_t lengths[3] = {1, 1, 1};
    if (MC_MUTANT == 1)
        (void)ow_destroy(published);
    r.storage = ow_construct(&staged, a, lengths);
    if (r.storage != OW_OK)
        return r;
    staged.block[0][0] = (int32_t)sum;
    staged.block[1][0] = 2;
    staged.block[2][0] = (int32_t)n;
    if (MC_MUTANT == 2)
    {
        *published = staged; /* leaks the old three owned blocks */
        r.storage = OW_OK;
    }
    else
        r.storage = ow_publish(published, &staged);
    r.committed = r.storage == OW_OK;
    if (!r.committed)
        (void)ow_destroy(&staged);
    return r;
}

enum msg_status mc_read_init(struct mc_read *r, struct co_async_port port,
                             size_t wanted, uint32_t generation,
                             unsigned char out[8], msg_receiver_fn receiver,
                             void *context, ow_cleanup_fn cleanup,
                             void *cleanup_context)
{
    if (!r || !out || !receiver || !cleanup || !port.read || !port.arm ||
        !port.disarm)
        return MSG_INVALID;
    struct mc_read value = {0};
    if (co_async_init(&value.frame, wanted, generation) != CO_OK)
        return MSG_INVALID;
    if (ow_defer(&value.cleanup, cleanup, cleanup_context, OW_ALWAYS) != OW_OK)
        return MSG_FAILED;
    value.port = port;
    value.output = out;
    msg_connect(&value.operation, receiver, context);
    if (msg_start(&value.operation) != MSG_OK)
        return MSG_FAILED;
    *r =
        value; /* No registration exists yet; this is the last permitted move. */
    return MSG_OK;
}

static enum msg_status finish(struct mc_read *r, enum msg_status status,
                              unsigned progress)
{
    r->terminal = true;
    if (MC_MUTANT != 4)
        (void)ow_finish(&r->cleanup, status == MSG_OK);
    /* Completion may free r. Return a local value; never read r afterwards. */
    enum msg_status delivered = msg_complete(&r->operation, status, progress);
    return delivered == MSG_OK ? status : delivered;
}

enum msg_status mc_read_poll(struct mc_read *r)
{
    if (!r || r->terminal)
        return MSG_INVALID;
    unsigned char data[8];
    size_t count = 0;
    int result = co_async_poll(&r->frame, r->port, data, &count);
    if (result == CO_PENDING)
        return MSG_BUSY;
    if (result == CO_DONE)
    {
        for (size_t i = 0; i < count; ++i)
            r->output[i] = data[i];
        return finish(r, MSG_OK, (unsigned)count);
    }
    return finish(r, MSG_FAILED, (unsigned)r->frame.length);
}

enum msg_status mc_read_cancel(struct mc_read *r)
{
    if (!r || r->terminal)
        return MSG_INVALID;
    (void)msg_stop_operation(&r->operation);
    unsigned progress = (unsigned)r->frame.length;
    if (MC_MUTANT != 3 && co_async_cancel(&r->frame, r->port) != CO_OK)
        return MSG_INVALID;
    return finish(r, MSG_CANCELLED, progress);
}
