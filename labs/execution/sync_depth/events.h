#ifndef CMASTERY_EVENTS_DEPTH_H
#define CMASTERY_EVENTS_DEPTH_H
#include "cmx.h"
enum { CDE_IDLE, CDE_PENDING, CDE_DONE };
typedef struct { unsigned phase; int result; uint32_t observed; } CDEOperation;
typedef struct {
  cmx_kernel *kernel;
  cmx_event event;
  CDEOperation operation[CMX_TASKS];
} CDEvents;
typedef struct { uint32_t epoch, serial; } CDECursor;
typedef struct { CDECursor current; } CDESequence;
int cde_init(CDEvents *, cmx_kernel *);
int cde_wait(CDEvents *, unsigned task, uint32_t mask, unsigned all,
             unsigned consume, cmx_deadline);
int cde_result(CDEvents *, unsigned task, uint32_t *observed);
int cde_set(CDEvents *, uint32_t bits);
int cde_clear(CDEvents *, uint32_t bits);
int cde_close(CDEvents *);
int cde_destroy(CDEvents *);
void cde_sequence_init(CDESequence *);
int cde_sequence_publish(CDESequence *);
int cde_sequence_read(const CDESequence *, CDECursor *, uint64_t *delta,
                       uint64_t *missed);
#endif
