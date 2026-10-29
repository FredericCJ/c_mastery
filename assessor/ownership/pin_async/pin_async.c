#include "pin_async.h"
#ifndef OPA_MUTATION
#define OPA_MUTATION 0
#endif
static enum ow_status admitted(const struct opa_machine *m, uint32_t ticket,
                               uint64_t owner)
{
    if (!m || owner != m->pin.thread)
        return OW_INVALID;
    if (ticket != m->async.generation)
        return OW_STALE;
    if (!m->pin.pending)
        return OW_INVALID;
    return OW_OK;
}

enum ow_status opa_init(struct opa_machine *m, const void *address,
                        uint64_t owner, uint64_t executor)
{
    if (!m || !address)
        return OW_INVALID;
    struct opa_machine stage = {0};
    enum ow_status q = ow_pin_init(&stage.pin, address, owner);
    if (q != OW_OK)
        return q;
    stage.executor = executor;
    *m = stage;
    return OW_OK;
}

enum ow_status opa_submit(struct opa_machine *m, const void *address,
                          uint64_t owner, uint32_t *out)
{
    if (!m || !out)
        return OW_INVALID;
    enum ow_status q = ow_pin_check(&m->pin, address, owner);
    if (q != OW_OK)
        return q;
    if (m->pin.pending)
        return OW_BUSY;
    if (m->async.generation == UINT32_MAX ||
        (m->async.releases == UINT32_MAX && OPA_MUTATION != 4))
        return OW_FULL;
    struct opa_machine stage = *m;
    uint32_t ticket = 0;
    if (f_submit(&stage.async, &ticket) != F_OK)
        return OW_INVALID;
    q = ow_pin_submit(&stage.pin, address, owner);
    if (q != OW_OK)
        return q;
    stage.notified = 0;
    stage.recovery = 0;
    stage.result = 0;
    *m = stage;
    *out = ticket;
    return OW_OK;
}

enum ow_status opa_cancel(struct opa_machine *m, uint32_t ticket,
                          uint64_t owner)
{
    enum ow_status q = admitted(m, ticket, owner);
    if (q != OW_OK)
        return q;
    if (m->async.state == F_CANCEL_REQUESTED)
        return OW_OK;
    struct opa_machine stage = *m;
    if (f_cancel(&stage.async, ticket) != F_PENDING)
        return OW_INVALID;
    q = ow_pin_cancel(&stage.pin, owner);
    if (q != OW_OK)
        return q;
    *m = stage;
    return OW_OK;
}

enum ow_status opa_timeout(struct opa_machine *m, uint32_t ticket,
                           uint64_t owner)
{
    enum ow_status q = opa_cancel(m, ticket, owner);
    if (q == OW_OK)
    {
        m->recovery = 1;
        if (OPA_MUTATION == 1)
            m->pin.pending = 0;
    }
    return q;
}

enum ow_status opa_notify(struct opa_machine *m, uint32_t ticket,
                          uint64_t executor, int32_t result)
{
    if (!m || (executor != m->executor && OPA_MUTATION != 3))
        return OW_INVALID;
    if (ticket != m->async.generation)
        return OW_STALE;
    if (!m->pin.pending || m->notified)
        return OW_INVALID;
    m->result = result;
    m->notified = 1;
    if (OPA_MUTATION == 2)
        m->pin.pending = 0;
    return OW_OK;
}

enum ow_status opa_finish(struct opa_machine *m, uint32_t ticket,
                          uint64_t owner, int32_t *out)
{
    if (!out)
        return OW_INVALID;
    enum ow_status q = admitted(m, ticket, owner);
    if (q != OW_OK)
        return q;
    if (!m->notified)
        return OW_BUSY;
    struct opa_machine stage = *m;
    if (f_complete(&stage.async, ticket) != F_OK)
        return OW_FULL;
    q = ow_pin_complete(&stage.pin, owner);
    if (q != OW_OK)
        return q;
    stage.recovery = 0;
    *m = stage;
    *out = stage.result;
    return OW_OK;
}

enum ow_status opa_rebind(struct opa_machine *m, const void *address,
                          uint64_t owner)
{
    if (!m)
        return OW_INVALID;
    return ow_pin_rebind(&m->pin, address, owner);
}
