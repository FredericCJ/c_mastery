#include "notifications_irq.h"
/* Deliberately incomplete target-port scaffold. */
uint32_t cdn_irq_save(void) { return 0; }
void cdn_irq_restore(uint32_t value) { (void)value; }
int cdn_irq_post(CDNHandle h,uint32_t n,CDNPost *p) { (void)h;(void)n;(void)p;return CMX_INVALID; }
int cdn_thread_wait(CDNHandle h,uint32_t n,cmx_deadline d) { (void)h;(void)n;(void)d;return CMX_INVALID; }
