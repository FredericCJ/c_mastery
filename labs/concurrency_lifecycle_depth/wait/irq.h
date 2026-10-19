#ifndef CMASTERY_WAIT_IRQ_H
#define CMASTERY_WAIT_IRQ_H
#include "wait.h"
uint32_t cw_irq_save(void);
void cw_irq_restore(uint32_t);
int cw_irq_begin(CWWorld *, unsigned, unsigned, cmx_deadline, unsigned,
                 CWToken *);
int cw_irq_post(CWWorld *, unsigned);
/* Public test hook: no block/reentry; runs while configurable IRQs are masked.
 */
void cw_irq_enrollment_hook(void);
#endif
