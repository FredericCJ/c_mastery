#ifndef CMASTERY_OWNERSHIP_PIN_HOST_H
#define CMASTERY_OWNERSHIP_PIN_HOST_H
#include "pin_async.h"
#include <pthread.h>
typedef void (*OpaReceiver)(void *,int32_t);
typedef struct {unsigned pending,recovery,notified,done;uint32_t generation,releases;} OpaObservation;
typedef struct {pthread_mutex_t mutex;pthread_cond_t condition;pthread_t owner,worker;OpaMachine core;int32_t *buffer;size_t count;int32_t value;OpaReceiver receiver;void *context;uint32_t ticket;unsigned ready,started,gate,done,joined;OwStatus worker_status;} OpaPort;
/* Fresh nonowning stable port; caller keeps buffer and contexts live through
 * actual joined completion. Host backend deliberately may write after timeout. */
OwStatus opa_port_init(OpaPort *,int32_t *,size_t);
OwStatus opa_port_start(OpaPort *,int32_t,OpaReceiver,void *,uint32_t *);
OwStatus opa_port_cancel(OpaPort *,uint32_t,int timeout_request);
OwStatus opa_port_allow_backend(OpaPort *,uint32_t);
OwStatus opa_port_observe(OpaPort *,OpaObservation *);
/* wait=0 refuses until the done flag; even then actual join can wait for exit.
 * wait=1 also waits for done. Neither notified nor done permits reuse before
 * actual pthread_join succeeds. */
OwStatus opa_port_join(OpaPort *,uint32_t,int wait,int32_t *);
OwStatus opa_port_rebind(OpaPort *,int32_t *,size_t);
OwStatus opa_port_destroy(OpaPort *);
#endif
