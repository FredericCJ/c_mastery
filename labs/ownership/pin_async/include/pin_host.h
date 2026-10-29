#ifndef CMASTERY_OWNERSHIP_PIN_HOST_H
#define CMASTERY_OWNERSHIP_PIN_HOST_H
#include "pin_async.h"
#include <pthread.h>
typedef void (*opa_receiver_fn)(void *,int32_t);
struct opa_observation {unsigned pending,recovery,notified,done;uint32_t generation,releases;};
struct opa_port {pthread_mutex_t mutex;pthread_cond_t condition;pthread_t owner,worker;struct opa_machine core;int32_t *buffer;size_t count;int32_t value;opa_receiver_fn receiver;void *context;uint32_t ticket;unsigned ready,started,gate,done,joined;enum ow_status worker_status;};
/* Fresh nonowning stable port; caller keeps buffer and contexts live through
 * actual joined completion. Host backend deliberately may write after timeout. */
enum ow_status opa_port_init(struct opa_port *,int32_t *,size_t);
enum ow_status opa_port_start(struct opa_port *,int32_t,opa_receiver_fn,void *,uint32_t *);
enum ow_status opa_port_cancel(struct opa_port *,uint32_t,int timeout_request);
enum ow_status opa_port_allow_backend(struct opa_port *,uint32_t);
enum ow_status opa_port_observe(struct opa_port *,struct opa_observation *);
/* wait=0 refuses until the done flag; even then actual join can wait for exit.
 * wait=1 also waits for done. Neither notified nor done permits reuse before
 * actual pthread_join succeeds. */
enum ow_status opa_port_join(struct opa_port *,uint32_t,int wait,int32_t *);
enum ow_status opa_port_rebind(struct opa_port *,int32_t *,size_t);
enum ow_status opa_port_destroy(struct opa_port *);
#endif
