#ifndef CMASTERY_ACTIVE_IMPORT_H
#define CMASTERY_ACTIVE_IMPORT_H
#include "event_follow.h"
#include "cmx.h"
typedef struct{cmx_kernel kernel;cmx_queue queue;PTMachine machine;PFLock lock;unsigned held,event,discarded;_Alignas(16) unsigned char stacks[4][256];} PATaskActive;
int pa_active_init(PATaskActive*,PFLock);
int pa_active_post(PATaskActive*,unsigned,unsigned);
int pa_active_step(PATaskActive*,unsigned,unsigned char*,size_t,size_t*);
int pa_active_discard(PATaskActive*,unsigned,unsigned*);
#endif
