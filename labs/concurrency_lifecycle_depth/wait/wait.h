#ifndef CMASTERY_GENERATIONAL_CMX_WAIT_H
#define CMASTERY_GENERATIONAL_CMX_WAIT_H
#include "cmx.h"

enum { CW_SEMAPHORE, CW_MUTEX };
enum { CW_NONE, CW_READY, CW_TIMEOUT, CW_CANCEL, CW_CLOSE };
enum { CW_STALE = CMX_UNIMPLEMENTED + 1 };
struct cw_object {
  cmx_sem semaphore;
  cmx_mutex mutex;
  uint32_t generation;
  unsigned kind, live;
  uint64_t posts, acquired;
};
struct cw_node {
  uint32_t sequence, object_generation;
  unsigned task_generation, object, linked, cause;
  int result;
  uint64_t terminals;
};
struct cw_world {
  cmx_kernel *kernel;
  struct cw_object objects[2];
  struct cw_node nodes[CMX_TASKS];
};
struct cw_token {
  struct cw_world *owner;
  unsigned task, object, task_generation;
  uint32_t object_generation, operation;
};

void cw_init(struct cw_world *, cmx_kernel *);
int cw_object_init(struct cw_world *, unsigned, unsigned kind, unsigned initial,
                   unsigned maximum);
int cw_begin(struct cw_world *, unsigned object, unsigned task, cmx_deadline,
             unsigned strict_deadline, struct cw_token *);
int cw_result(struct cw_token, int *result, unsigned *cause);
int cw_post(struct cw_world *, unsigned object);
int cw_ready_callback(struct cw_token);
int cw_timeout(struct cw_token, uint32_t now);
int cw_cancel(struct cw_token);
int cw_unlock(struct cw_world *, unsigned object, unsigned task);
int cw_close(struct cw_world *, unsigned object);
int cw_exit_reap(struct cw_world *, unsigned task);
#endif
