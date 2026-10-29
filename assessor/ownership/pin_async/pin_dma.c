#include "pin_dma.h"
#ifndef OPA_MUTATION
#define OPA_MUTATION 0
#endif
enum ow_status opa_dma_init(struct opa_dma *d, const void *scope,
                            struct pd_io io)
{
    if (!d || !scope || !io.configure || !io.enable || !io.stop ||
        !io.snapshot || !io.visibility)
    {
        return OW_INVALID;
    }
    struct opa_dma stage = {0};
    enum ow_status q = opa_init(&stage.owner, scope, 7, 9);
    if (q != OW_OK)
    {
        return q;
    }
    stage.io = io;
    *d = stage;
    return OW_OK;
}

enum ow_status opa_dma_submit(struct opa_dma *d,
                              const struct pd_extent *extents, unsigned count,
                              unsigned board, unsigned cached, uint32_t *out)
{
    if (!d || !out)
    {
        return OW_INVALID;
    }
    if (d->owner.pin.pending || d->transfer.phase)
    {
        return OW_BUSY;
    }
    uint32_t total = 0;
    if (pd_validate(board, cached, extents, count, &total) != PD_OK)
    {
        return OW_INVALID;
    }
    struct opa_machine old = d->owner;
    uint32_t ticket = 0;
    enum ow_status q = opa_submit(&d->owner, d->owner.pin.address, 7, &ticket);
    if (q != OW_OK)
    {
        return q;
    }
    /* Publish ownership before enable. Configure refusal cannot start a transfer. */
    int status =
        pd_submit(&d->transfer, d->io, board, cached, extents, count, ticket);
    if (status != PD_OK)
    {
        d->owner = old;
        return status == PD_BUSY ? OW_BUSY : OW_INVALID;
    }
    d->finalized = 0;
    *out = ticket;
    return OW_OK;
}

enum ow_status opa_dma_timeout(struct opa_dma *d, uint32_t ticket)
{
    if (!d)
    {
        return OW_INVALID;
    }
    enum ow_status q = opa_timeout(&d->owner, ticket, 7);
    if (q != OW_OK)
    {
        return q;
    }
    int status = pd_cancel(&d->transfer, d->io, ticket);
    return status == PD_BUSY ? OW_OK : OW_INVALID;
}

enum ow_status opa_dma_poll(struct opa_dma *d, uint32_t ticket,
                            struct pd_result *out)
{
    if (!d || !out)
    {
        return OW_INVALID;
    }
    if (ticket != d->owner.async.generation || !d->owner.pin.pending)
    {
        return OW_STALE;
    }
    if (!d->finalized)
    {
        int status = pd_poll(&d->transfer, d->io, ticket);
        if (status == PD_BUSY)
        {
            if (OPA_MUTATION == 5)
            {
                int32_t ignored = 0;
                (void)opa_notify(&d->owner, ticket, 9, 0);
                (void)opa_finish(&d->owner, ticket, 7, &ignored);
                return OW_OK;
            }
            return OW_BUSY;
        }
        if (status != PD_OK)
        {
            return OW_INVALID;
        }
        if (pd_finalize(&d->transfer, d->io, ticket, &d->result) != PD_OK)
        {
            return OW_INVALID;
        }
        d->finalized = 1;
    }
    /* A retained finalized result permits retry without a second Pd finalize. */
    enum ow_status q = OW_OK;
    if (!d->owner.notified)
    {
        q = opa_notify(&d->owner, ticket, 9, (int32_t)d->result.status);
    }
    if (q != OW_OK)
    {
        return q;
    }
    int32_t ignored = 0;
    q = opa_finish(&d->owner, ticket, 7, &ignored);
    if (q != OW_OK)
    {
        return q;
    }
    *out = d->result;
    return OW_OK;
}
