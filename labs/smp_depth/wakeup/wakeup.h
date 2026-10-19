#ifndef SMP_WAKEUP_H
#define SMP_WAKEUP_H
#include "queues.h"
enum { SW_SIGNAL=7,SW_MOVING,SW_BUSY };
typedef struct {int (*ring)(void *,unsigned);void (*ack)(void *,unsigned);void *context;} SwPort;
typedef struct {SqWorld queues;SlLock control;SwPort port;unsigned bell[SQ_CPUS];atomic_uint work[SQ_CPUS];} SwWorld;
/* Stable initialized world; all queue operations after creation use these
 * wrappers. One caller per actor CPU, no wrapper calls from an ISR. The
 * fixed port callbacks are bounded and nonreentrant, invoked under control.
 * Interrupt acknowledgement is distinct from clearing the software doorbell.
 * SW_SIGNAL means work was admitted but notification failed: retain and retry.
 */
int sw_init(SwWorld *,unsigned cpus,unsigned capacity,SwPort);
int sw_create(SwWorld *,unsigned actor,unsigned home,unsigned slot,SqHandle *,SqEvent *);
int sw_wake(SwWorld *,unsigned actor,unsigned home,SqHandle,SqEvent *,SlStats *);
int sw_dispatch(SwWorld *,unsigned cpu,SqHandle *,SqEvent *,SlStats *);
int sw_finish(SwWorld *,unsigned cpu,SqHandle,unsigned next,SqEvent *,SlStats *);
int sw_transfer_begin(SwWorld *,unsigned actor,unsigned source,unsigned destination,SqHandle,SqTransfer *,SqEvent *,SlStats *);
int sw_transfer_end(SwWorld *,unsigned actor,SqTransfer,int commit,SqEvent *,SlStats *);
int sw_retry(SwWorld *,unsigned actor,unsigned destination,SlStats *);
/* The selected ISA port must mask local interrupts before this check and use
 * a proven atomic enable/sleep sequence after SQ_IDLE. The mask argument is
 * a trusted port assertion, not a substitute for actually reading IF. */
int sw_prepare_idle(SwWorld *,unsigned cpu,int masked,SlStats *);
#endif
