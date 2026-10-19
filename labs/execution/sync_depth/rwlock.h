#ifndef CMASTERY_RWLOCK_DEPTH_H
#define CMASTERY_RWLOCK_DEPTH_H
#include "cmx.h"
enum{CDR_IDLE,CDR_PENDING,CDR_TERMINAL,CDR_HELD};
enum{CDR_READ,CDR_WRITE};
enum{CDR_STALE=CMX_UNIMPLEMENTED+1,CDR_RETIRED};
typedef struct{unsigned phase,mode,generation;uint32_t serial;int result;} CDRSlot;
typedef struct{
  cmx_kernel *kernel;cmx_rwlock raw;CDRSlot slot[CMX_TASKS];
  uint32_t registered;unsigned read_limit,live,closing;
} CDRW;
typedef struct{CDRW *owner;unsigned task,generation;} CDRHandle;
typedef struct{CDRHandle handle;uint32_t serial;} CDRTicket;
int cdr_init(CDRW *,cmx_kernel *,unsigned read_limit);
int cdr_register(CDRW *,unsigned,CDRHandle *);
int cdr_begin(CDRHandle,unsigned mode,cmx_deadline,CDRTicket *);
int cdr_result(CDRTicket);
int cdr_cancel(CDRTicket);
int cdr_unlock(CDRHandle);
int cdr_close(CDRW *);
int cdr_release(CDRHandle *);
int cdr_destroy(CDRW *);
#endif
