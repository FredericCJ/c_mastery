#include "irq.h"
/* Deliberately unsolved target wrapper. */
uint32_t cw_irq_save(void) { return 0; }
void cw_irq_restore(uint32_t mask) { (void)mask; }
int cw_irq_begin(struct cw_world *w, unsigned o, unsigned t, cmx_deadline d, unsigned s,
                 struct cw_token *h) {
  (void)w;
  (void)o;
  (void)t;
  (void)d;
  (void)s;
  (void)h;
  return CMX_UNIMPLEMENTED;
}
int cw_irq_post(struct cw_world *w, unsigned o) {
  (void)w;
  (void)o;
  return CMX_UNIMPLEMENTED;
}
