#ifndef CMASTERY_FIBER_FOLLOW_H
#define CMASTERY_FIBER_FOLLOW_H
#include "fiber.h"
typedef struct CFFiber CFFiber;
typedef void (*CFFiberFn)(CFFiber *,void *);
struct CFFiber{COFiber fiber;void *mapping;size_t mapping_size,page_size;unsigned cancel;CFFiberFn run;void *argument;};
int cf_fiber_create(CFFiber *,size_t,CFFiberFn,void *);
int cf_fiber_resume(CFFiber *);
int cf_fiber_suspend(CFFiber *);
int cf_fiber_cancel(CFFiber *);
int cf_fiber_destroy(CFFiber *);
#endif
