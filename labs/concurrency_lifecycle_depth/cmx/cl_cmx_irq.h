#ifndef CMASTERY_CMX_COPIED_QUEUE_IRQ_PORT_H
#define CMASTERY_CMX_COPIED_QUEUE_IRQ_PORT_H
#include "cl_cmx.h"
#include "irq.h"
int cc_irq_receive(CCHandle,size_t,cmx_deadline,CCTicket *);
int cc_irq_publish(CCPort,uint32_t,uint32_t,const unsigned char *,size_t);
/* Privileged thread-mode join for this dedicated software-only IRQ0 source.
 * Disables, clears pending and checks inactive before dropping the lease.
 * This is not a peripheral DMA/device quiescence operation. */
int cc_irq_join0(CCPort *);
void cc_irq_enrollment_hook(void);
void cc_irq_join_hook(void);
#endif
