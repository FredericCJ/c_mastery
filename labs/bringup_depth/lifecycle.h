#ifndef BU_LIFECYCLE_H
#define BU_LIFECYCLE_H
#include "bringup.h"
/* Serialized synthetic engine; events are observations, not hardware requests.
 */
struct bu_drain {
  unsigned callbacks, remaining, stopped, acknowledged, expired, transferred;
};
enum bu_result bu_drain_begin(struct bu_drain *d, unsigned callbacks, unsigned poll_limit);
enum bu_result bu_drain_step(struct bu_drain *d, unsigned stop_ack, unsigned callbacks_done);
/* Machine-owned supervisor, with task-owned bytes separate from retained state.
 */
struct bu_task {
  unsigned phase, generation, pending;
  int last_result;
  unsigned char task_bytes[8];
};
enum bu_result bu_task_start(struct bu_task *t);
enum bu_result bu_task_return(struct bu_task *t, unsigned generation, int result,
                        unsigned pending);
enum bu_result bu_task_drained(struct bu_task *t, unsigned generation);
#endif
