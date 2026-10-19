#ifndef CMASTERY_COPIED_QUEUE_CMX_PORT_H
#define CMASTERY_COPIED_QUEUE_CMX_PORT_H
#include "cl.h"
#include "cmx.h"

enum { CC_BLOCKED=CL_UNIMPLEMENTED+1,CC_TIMEOUT,CC_CANCELLED,CC_RETIRED,CC_INVALID };
enum { CC_IDLE,CC_WAITING,CC_TERMINAL,CC_REACQUIRED };
enum { CC_SEND,CC_RECEIVE,CC_CONDITION };
typedef struct {
  CLLease lease;
  uint32_t serial;
  unsigned generation,registered,phase,kind,order;
  const unsigned char *borrowed;
  size_t length,capacity;
  uint32_t producer,sequence;
  CLMessage received;
  cmx_latch completion;
  cmx_mutex *reacquire;
  int result;
} CCNode;
typedef struct {
  cmx_kernel *kernel;
  CLLife life;
  CLQueue bytes;
  CLMessage storage[2];
  CCNode nodes[CMX_TASKS];
} CCQueue;
typedef struct { CCQueue *owner;unsigned task,generation;CLLease lease; } CCHandle;
typedef struct { CCHandle handle;uint32_t serial; } CCTicket;
typedef struct { CCQueue *owner;CLLease lease; } CCPort;

/* Every core entry and CMX progress shares one exclusion domain. Live stable
 * owner storage is a precondition even when a handle is stale. */
int cc_init(CCQueue *,cmx_kernel *);
int cc_register(CCQueue *,unsigned task,CCHandle *);
int cc_send(CCHandle,uint32_t producer,uint32_t sequence,const unsigned char *,
            size_t length,cmx_deadline,CCTicket *);
int cc_receive(CCHandle,size_t capacity,cmx_deadline,CCTicket *);
int cc_result(CCTicket,CLMessage *received);
int cc_cancel(CCTicket);
int cc_progress(CCQueue *);
int cc_close(CCQueue *,unsigned abort_queued);
int cc_release(CCHandle *);
int cc_destroy(CCQueue *);
int cc_restart(CCQueue *);
/* Reacquired mutex remains owned even when the wait result is an error. */
int cc_condition(CCHandle,cmx_cond *,cmx_mutex *,cmx_deadline,CCTicket *);
int cc_condition_unlock(CCHandle);
/* The port lease is an implicit callback reference. Its release requires an
 * actual external join/IRQ quiescence proof; the serialized C core cannot
 * infer that a future caller has stopped. */
int cc_port_acquire(CCQueue *,CCPort *);
int cc_port_send(CCPort,uint32_t producer,uint32_t sequence,const unsigned char *,size_t length);
int cc_port_release_after_quiescence(CCPort *);
#endif
