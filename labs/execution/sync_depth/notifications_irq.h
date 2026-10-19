#ifndef CMASTERY_NOTIFICATIONS_IRQ_H
#define CMASTERY_NOTIFICATIONS_IRQ_H
#include "notifications.h"
#define CDN_KERNEL_THRESHOLD 0x80u
uint32_t cdn_irq_save(void);
void cdn_irq_restore(uint32_t);
int cdn_irq_post(CDNHandle, uint32_t, CDNPost *);
int cdn_thread_wait(CDNHandle, uint32_t, cmx_deadline);
#endif
