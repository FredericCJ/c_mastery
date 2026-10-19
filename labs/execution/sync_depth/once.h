#ifndef CMASTERY_ONCE_DEPTH_H
#define CMASTERY_ONCE_DEPTH_H
#include "cmx.h"
#include "bridge.h"
enum{CDO_IDLE,CDO_OWNER,CDO_PENDING,CDO_DONE};
enum{CDO_INIT=CMX_UNIMPLEMENTED+1,CDO_FAILED,CDO_STALE,CDO_RETIRED,CDO_AGAIN};
typedef struct{uint32_t value;} CDOCell;
typedef CDOCell *(*CDOAllocate)(void *);
typedef void (*CDODrop)(void *,CDOCell *);
typedef struct{uint32_t entries[8],count,checksum;} CDOTable;
typedef struct{unsigned phase,task_generation;uint32_t epoch,serial;int result;} CDOSlot;
typedef struct{
  cmx_kernel *kernel;
  cmx_once wait;
  MBOnce publication;
  CDOTable table;
  CDOSlot slot[3];
  CDOCell *resource[3];
  CDOAllocate allocate;
  CDODrop drop;
  void *allocator_context;
  uint32_t epoch,attempts;
  unsigned registered,stage,resources,resetting,closing,live;
} CDOnce;
typedef struct{CDOnce *owner;unsigned task;uint32_t epoch;} CDOHandle;
typedef struct{CDOHandle handle;uint32_t epoch,serial;} CDOTicket;
int cdo_init(CDOnce *,cmx_kernel *,CDOAllocate,CDODrop,void *);
int cdo_register(CDOnce *,unsigned task,CDOHandle *);
int cdo_enter(CDOHandle,cmx_deadline,CDOTicket *);
int cdo_allocate(CDOHandle);
int cdo_write(CDOHandle,unsigned index,uint32_t value);
int cdo_seal(CDOHandle);
int cdo_finish(CDOHandle,unsigned success);
int cdo_result(CDOTicket,CDOTable *);
int cdo_read(CDOHandle,CDOTable *);
int cdo_cancel(CDOTicket);
int cdo_prepare_reset(CDOnce *);
int cdo_release(CDOHandle *);
int cdo_reset(CDOnce *);
int cdo_close(CDOnce *);
int cdo_destroy(CDOnce *);
#endif
