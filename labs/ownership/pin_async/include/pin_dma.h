#ifndef CMASTERY_OWNERSHIP_PIN_DMA_H
#define CMASTERY_OWNERSHIP_PIN_DMA_H
#include "pin_async.h"
#include "dma.h"
typedef struct {OpaMachine owner;PdTransfer transfer;PdIo io;PdResult result;unsigned finalized;} OpaDma;
/* scope is a live nonmovable lease-owner identity covering ALL admitted Pd
 * payloads. Numeric bank validation does not establish those real allocations. */
OwStatus opa_dma_init(OpaDma *,const void *scope,PdIo);
OwStatus opa_dma_submit(OpaDma *,const PdExtent *,unsigned,unsigned board,unsigned cached,uint32_t *);
OwStatus opa_dma_timeout(OpaDma *,uint32_t);
OwStatus opa_dma_poll(OpaDma *,uint32_t,PdResult *);
#endif
