#ifndef CMASTERY_OWNERSHIP_PIN_DMA_H
#define CMASTERY_OWNERSHIP_PIN_DMA_H
#include "pin_async.h"
#include "dma.h"

struct opa_dma
{
    struct opa_machine owner;
    struct pd_transfer transfer;
    struct pd_io io;
    struct pd_result result;
    unsigned finalized;
};
/* scope is a live nonmovable lease-owner identity covering ALL admitted Pd
 * payloads. Numeric bank validation does not establish those real allocations. */
enum ow_status opa_dma_init(struct opa_dma *, const void *scope, struct pd_io);
enum ow_status opa_dma_submit(struct opa_dma *, const struct pd_extent *,
                              unsigned, unsigned board, unsigned cached,
                              uint32_t *);
enum ow_status opa_dma_timeout(struct opa_dma *, uint32_t);
enum ow_status opa_dma_poll(struct opa_dma *, uint32_t, struct pd_result *);
#endif
