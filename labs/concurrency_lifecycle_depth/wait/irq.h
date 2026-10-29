#ifndef CMASTERY_WAIT_IRQ_H
#define CMASTERY_WAIT_IRQ_H
#include "wait.h"
uint32_t cw_irq_save(void);
void cw_irq_restore(uint32_t);
int cw_irq_begin(struct cw_world *, unsigned, unsigned, cmx_deadline, unsigned,
                 struct cw_token *);
int cw_irq_post(struct cw_world *, unsigned);
/* Public test hook: no block/reentry; runs while configurable IRQs are masked.
 */
void cw_irq_enrollment_hook(void);
#endif
