#ifndef SMP_WAKEUP_H
#define SMP_WAKEUP_H
#include "queues.h"
enum { SW_SIGNAL=7,SW_MOVING,SW_BUSY };
struct sw_port {int (*ring)(void *,unsigned);void (*ack)(void *,unsigned);void *context;};
struct sw_world {struct sq_world queues;struct sl_lock control;struct sw_port port;unsigned bell[SQ_CPUS];atomic_uint work[SQ_CPUS];};
/* Stable initialized world; all queue operations after creation use these
 * wrappers. One caller per actor CPU, no wrapper calls from an ISR. The
 * fixed port callbacks are bounded and nonreentrant, invoked under control.
 * Interrupt acknowledgement is distinct from clearing the software doorbell.
 * SW_SIGNAL means work was admitted but notification failed: retain and retry.
 */
int sw_init(struct sw_world *,unsigned cpus,unsigned capacity,struct sw_port);
int sw_create(struct sw_world *,unsigned actor,unsigned home,unsigned slot,struct sq_handle *,struct sq_event *);
int sw_wake(struct sw_world *,unsigned actor,unsigned home,struct sq_handle,struct sq_event *,struct sl_stats *);
int sw_dispatch(struct sw_world *,unsigned cpu,struct sq_handle *,struct sq_event *,struct sl_stats *);
int sw_finish(struct sw_world *,unsigned cpu,struct sq_handle,unsigned next,struct sq_event *,struct sl_stats *);
int sw_transfer_begin(struct sw_world *,unsigned actor,unsigned source,unsigned destination,struct sq_handle,struct sq_transfer *,struct sq_event *,struct sl_stats *);
int sw_transfer_end(struct sw_world *,unsigned actor,struct sq_transfer,int commit,struct sq_event *,struct sl_stats *);
int sw_retry(struct sw_world *,unsigned actor,unsigned destination,struct sl_stats *);
/* The selected ISA port must mask local interrupts before this check and use
 * a proven atomic enable/sleep sequence after SQ_IDLE. The mask argument is
 * a trusted port assertion, not a substitute for actually reading IF. */
int sw_prepare_idle(struct sw_world *,unsigned cpu,int masked,struct sl_stats *);
#endif
