#ifndef CMASTERY_CMX_COPIED_QUEUE_IRQ_PORT_H
#define CMASTERY_CMX_COPIED_QUEUE_IRQ_PORT_H
#include "cl_cmx.h"
#include "irq.h"
int cc_irq_receive(struct cc_handle, size_t, cmx_deadline, struct cc_ticket *);
int cc_irq_publish(struct cc_port, uint32_t, uint32_t, const unsigned char *,
                   size_t);
/* Privileged thread-mode join for this dedicated software-only IRQ0 source.
 * Disables, clears pending and checks inactive before dropping the lease.
 * This is not a peripheral DMA/device quiescence operation. */
int cc_irq_join0(struct cc_port *);
void cc_irq_enrollment_hook(void);
void cc_irq_join_hook(void);
#endif
