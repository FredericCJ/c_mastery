#ifndef CMASTERY_CMX_COPIED_QUEUE_TASK_PORT_H
#define CMASTERY_CMX_COPIED_QUEUE_TASK_PORT_H
#include "cl_cmx.h"
#include "cmx_port.h"
/* Synchronous calls on the retained actual PSP-stack runtime. Blocking entry
 * requires current registered thread, every mask zero, and the same machine. */
int cc_task_send(CCHandle,uint32_t,uint32_t,const unsigned char *,size_t,cmx_deadline);
int cc_task_receive(CCHandle,size_t,cmx_deadline,CLMessage *);
int cc_task_condition(CCHandle,cmx_cond *,cmx_mutex *,cmx_deadline);
int cc_task_condition_unlock(CCHandle);
int cc_task_close(CCQueue *,unsigned abort_queued);
#endif
