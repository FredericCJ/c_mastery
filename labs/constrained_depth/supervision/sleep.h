#ifndef CMASTERY_CONSTRAINED_SLEEP_H
#define CMASTERY_CONSTRAINED_SLEEP_H
#include <stdint.h>
/* Selected original AN386 guest: Thread mode, ordinary Sleep, IRQ8/9 owned;
 * BASEPRI/FAULTMASK zero. Restore incoming PRIMASK and SCR. queued is a live
 * ISR-shared word read only while masked. Test hook may pend owned IRQs/SEV.
 * 0=one admitted idle attempt,1=invalid environment,2=unsolved. Recheck queued
 * afterwards; any return may be spurious and does not promise useful work. */
unsigned cs_idle(unsigned event, volatile uint32_t *queued,
                 void (*hook)(unsigned));
#endif
