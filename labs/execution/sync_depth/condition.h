#ifndef CMASTERY_SYNC_DEPTH_CONDITION_H
#define CMASTERY_SYNC_DEPTH_CONDITION_H
#include "cmx.h"
enum { CDQ_IDLE, CDQ_LOCK, CDQ_CONDITION, CDQ_DONE };
enum { CDQ_PUT, CDQ_GET };
struct cdq_request {
  unsigned phase, operation;
  uint32_t value;
  cmx_deadline deadline;
  int result;
};
struct cd_queue {
  cmx_kernel *kernel;
  cmx_mutex mutex;
  cmx_cond changed;
  uint32_t values[2];
  unsigned head, count, capacity, broadcast, closed, destroyed;
  uint64_t accepted, claimed, consumed;
  struct cdq_request request[CMX_TASKS];
};
int cdq_init(struct cd_queue *, cmx_kernel *, unsigned capacity, unsigned broadcast);
int cdq_begin(struct cd_queue *, unsigned task, unsigned operation, uint32_t value,
              cmx_deadline);
int cdq_begin_locked(struct cd_queue *, unsigned task, unsigned operation,
                     uint32_t value, cmx_deadline);
int cdq_resume(struct cd_queue *, unsigned task);
int cdq_result(struct cd_queue *, unsigned task, int *result, uint32_t *value);
int cdq_pulse(struct cd_queue *, cmx_mutex *held, unsigned task, unsigned broadcast);
int cdq_put_locked(struct cd_queue *, unsigned task, uint32_t value);
int cdq_get_locked(struct cd_queue *, unsigned task, uint32_t *value);
int cdq_shutdown(struct cd_queue *, unsigned task);
int cdq_destroy(struct cd_queue *);
#endif
