#ifndef CMASTERY_COPIED_QUEUE_CMX_PORT_H
#define CMASTERY_COPIED_QUEUE_CMX_PORT_H
#include "cl.h"
#include "cmx.h"

enum { CC_BLOCKED=CL_UNIMPLEMENTED+1,CC_TIMEOUT,CC_CANCELLED,CC_RETIRED,CC_INVALID };
enum { CC_IDLE,CC_WAITING,CC_TERMINAL,CC_REACQUIRED };
enum { CC_SEND,CC_RECEIVE,CC_CONDITION };
struct cc_node {
  struct cl_lease lease;
  uint32_t serial;
  unsigned generation,registered,phase,kind,order;
  const unsigned char *borrowed;
  size_t length,capacity;
  uint32_t producer,sequence;
  struct cl_message received;
  cmx_latch completion;
  cmx_mutex *reacquire;
  int result;
};
struct cc_queue {
  cmx_kernel *kernel;
  struct cl_life life;
  struct cl_queue bytes;
  struct cl_message storage[2];
  struct cc_node nodes[CMX_TASKS];
};
struct cc_handle { struct cc_queue *owner;unsigned task,generation;struct cl_lease lease; };
struct cc_ticket { struct cc_handle handle;uint32_t serial; };
struct cc_port { struct cc_queue *owner;struct cl_lease lease; };

/* Every core entry and CMX progress shares one exclusion domain. Live stable
 * owner storage is a precondition even when a handle is stale. */
int cc_init(struct cc_queue *,cmx_kernel *);
int cc_register(struct cc_queue *,unsigned task,struct cc_handle *);
int cc_send(struct cc_handle,uint32_t producer,uint32_t sequence,const unsigned char *,
            size_t length,cmx_deadline,struct cc_ticket *);
int cc_receive(struct cc_handle,size_t capacity,cmx_deadline,struct cc_ticket *);
int cc_result(struct cc_ticket,struct cl_message *received);
int cc_cancel(struct cc_ticket);
int cc_progress(struct cc_queue *);
int cc_close(struct cc_queue *,unsigned abort_queued);
int cc_release(struct cc_handle *);
int cc_destroy(struct cc_queue *);
int cc_restart(struct cc_queue *);
/* Reacquired mutex remains owned even when the wait result is an error. */
int cc_condition(struct cc_handle,cmx_cond *,cmx_mutex *,cmx_deadline,struct cc_ticket *);
int cc_condition_unlock(struct cc_handle);
/* The port lease is an implicit callback reference. Its release requires an
 * actual external join/IRQ quiescence proof; the serialized C core cannot
 * infer that a future caller has stopped. */
int cc_port_acquire(struct cc_queue *,struct cc_port *);
int cc_port_send(struct cc_port,uint32_t producer,uint32_t sequence,const unsigned char *,size_t length);
int cc_port_release_after_quiescence(struct cc_port *);
#endif
