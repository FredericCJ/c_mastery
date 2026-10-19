#ifndef CMASTERY_SYNC_DEPTH_CONDITION_H
#define CMASTERY_SYNC_DEPTH_CONDITION_H
#include "cmx.h"
enum { CDQ_IDLE, CDQ_LOCK, CDQ_CONDITION, CDQ_DONE };
enum { CDQ_PUT, CDQ_GET };
typedef struct {
  unsigned phase, operation;
  uint32_t value;
  cmx_deadline deadline;
  int result;
} CDQRequest;
typedef struct {
  cmx_kernel *kernel;
  cmx_mutex mutex;
  cmx_cond changed;
  uint32_t values[2];
  unsigned head, count, capacity, broadcast, closed, destroyed;
  uint64_t accepted, claimed, consumed;
  CDQRequest request[CMX_TASKS];
} CDQueue;
int cdq_init(CDQueue *, cmx_kernel *, unsigned capacity, unsigned broadcast);
int cdq_begin(CDQueue *, unsigned task, unsigned operation, uint32_t value,
              cmx_deadline);
int cdq_begin_locked(CDQueue *, unsigned task, unsigned operation,
                     uint32_t value, cmx_deadline);
int cdq_resume(CDQueue *, unsigned task);
int cdq_result(CDQueue *, unsigned task, int *result, uint32_t *value);
int cdq_pulse(CDQueue *, cmx_mutex *held, unsigned task, unsigned broadcast);
int cdq_put_locked(CDQueue *, unsigned task, uint32_t value);
int cdq_get_locked(CDQueue *, unsigned task, uint32_t *value);
int cdq_shutdown(CDQueue *, unsigned task);
int cdq_destroy(CDQueue *);
#endif
