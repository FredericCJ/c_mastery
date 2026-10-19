#ifndef CMASTERY_GENERATIONAL_CMX_WAIT_H
#define CMASTERY_GENERATIONAL_CMX_WAIT_H
#include "cmx.h"

enum { CW_SEMAPHORE, CW_MUTEX };
enum { CW_NONE, CW_READY, CW_TIMEOUT, CW_CANCEL, CW_CLOSE };
enum { CW_STALE = CMX_UNIMPLEMENTED + 1 };
typedef struct {
  cmx_sem semaphore;
  cmx_mutex mutex;
  uint32_t generation;
  unsigned kind, live;
  uint64_t posts, acquired;
} CWObject;
typedef struct {
  uint32_t sequence, object_generation;
  unsigned task_generation, object, linked, cause;
  int result;
  uint64_t terminals;
} CWNode;
typedef struct {
  cmx_kernel *kernel;
  CWObject objects[2];
  CWNode nodes[CMX_TASKS];
} CWWorld;
typedef struct {
  CWWorld *owner;
  unsigned task, object, task_generation;
  uint32_t object_generation, operation;
} CWToken;

void cw_init(CWWorld *, cmx_kernel *);
int cw_object_init(CWWorld *, unsigned, unsigned kind, unsigned initial,
                   unsigned maximum);
int cw_begin(CWWorld *, unsigned object, unsigned task, cmx_deadline,
             unsigned strict_deadline, CWToken *);
int cw_result(CWToken, int *result, unsigned *cause);
int cw_post(CWWorld *, unsigned object);
int cw_ready_callback(CWToken);
int cw_timeout(CWToken, uint32_t now);
int cw_cancel(CWToken);
int cw_unlock(CWWorld *, unsigned object, unsigned task);
int cw_close(CWWorld *, unsigned object);
int cw_exit_reap(CWWorld *, unsigned task);
#endif
