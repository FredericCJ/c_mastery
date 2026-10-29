#include "irq.h"
#ifndef CW_IRQ_MUTANT
#define CW_IRQ_MUTANT 0
#endif
uint32_t cw_irq_save(void) {
  uint32_t previous;
  if (CW_IRQ_MUTANT == 2)
    __asm__ volatile("mrs %0,primask" : "=r"(previous)::"memory");
  else
    __asm__ volatile("mrs %0,primask\ncpsid i" : "=r"(previous)::"memory");
  return previous;
}
void cw_irq_restore(uint32_t previous) {
  if (CW_IRQ_MUTANT == 1)
    previous = 0;
  __asm__ volatile("msr primask,%0\ndsb\nisb" ::"r"(previous) : "memory");
}
int cw_irq_begin(struct cw_world *world, unsigned object, unsigned task,
                 cmx_deadline deadline, unsigned strict, struct cw_token *token) {
  uint32_t previous = cw_irq_save();
  cw_irq_enrollment_hook();
  int result = cw_begin(world, object, task, deadline, strict, token);
  cw_irq_restore(previous);
  return result;
}
int cw_irq_post(struct cw_world *world, unsigned object) {
  uint32_t previous = cw_irq_save();
  int result = cw_post(world, object);
  cw_irq_restore(previous);
  return result;
}
