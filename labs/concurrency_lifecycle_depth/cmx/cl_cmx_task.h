#ifndef CMASTERY_CMX_COPIED_QUEUE_TASK_PORT_H
#define CMASTERY_CMX_COPIED_QUEUE_TASK_PORT_H
#include "cl_cmx.h"
#include "cmx_port.h"
/* Synchronous calls on the retained actual PSP-stack runtime. Blocking entry
 * requires current registered thread, every mask zero, and the same machine. */
int cc_task_send(struct cc_handle,uint32_t,uint32_t,const unsigned char *,size_t,cmx_deadline);
int cc_task_receive(struct cc_handle,size_t,cmx_deadline,struct cl_message *);
int cc_task_condition(struct cc_handle,cmx_cond *,cmx_mutex *,cmx_deadline);
int cc_task_condition_unlock(struct cc_handle);
int cc_task_close(struct cc_queue *,unsigned abort_queued);
#endif
