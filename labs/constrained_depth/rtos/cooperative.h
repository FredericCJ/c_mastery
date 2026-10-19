#ifndef CMASTERY_CONSTRAINED_RTOS_COOPERATIVE_H
#define CMASTERY_CONSTRAINED_RTOS_COOPERATIVE_H
#include "ownership.h"
typedef struct {unsigned jobs,sum,steps,blocked,wakes;} CrCoopReport;
/* Deterministic two-task backend. required_features bit0 preemption, bit1 ISR:
 * either is explicitly unsupported. Stop closes admission after stop_after
 * accepted jobs, then drains accepted payloads and quiesces both continuations.
 * Output is published only after all internal owners retire. */
int cr_cooperative(unsigned,unsigned,unsigned,CrCoopReport *);
#endif
