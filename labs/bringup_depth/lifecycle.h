#ifndef BU_LIFECYCLE_H
#define BU_LIFECYCLE_H
#include "bringup.h"
/* Serialized synthetic engine; events are observations, not hardware requests.
 */
typedef struct {
  unsigned callbacks, remaining, stopped, acknowledged, expired, transferred;
} BuDrain;
BuResult bu_drain_begin(BuDrain *d, unsigned callbacks, unsigned poll_limit);
BuResult bu_drain_step(BuDrain *d, unsigned stop_ack, unsigned callbacks_done);
/* Machine-owned supervisor, with task-owned bytes separate from retained state.
 */
typedef struct {
  unsigned phase, generation, pending;
  int last_result;
  unsigned char task_bytes[8];
} BuTask;
BuResult bu_task_start(BuTask *t);
BuResult bu_task_return(BuTask *t, unsigned generation, int result,
                        unsigned pending);
BuResult bu_task_drained(BuTask *t, unsigned generation);
#endif
