#ifndef CMASTERY_RWPHASE_DEPTH_H
#define CMASTERY_RWPHASE_DEPTH_H
#include "cmx.h"
enum{CPF_IDLE,CPF_PENDING,CPF_TERMINAL,CPF_HELD};
enum{CPF_READ,CPF_WRITE,CPF_UPGRADE};
enum{CPF_STALE=CMX_UNIMPLEMENTED+1,CPF_RETIRED};
typedef struct{
  unsigned phase,mode,generation;uint32_t serial,arrival;int result;
  cmx_latch completion;
} CPFSlot;
typedef struct{
  cmx_kernel *kernel;CPFSlot slot[CMX_TASKS];
  uint32_t registered,readers,admissions,phases;
  int writer,upgrader;unsigned last_phase,closing,live;
} CPF;
typedef struct{CPF *owner;unsigned task,generation;} CPFHandle;
typedef struct{CPFHandle handle;uint32_t serial;} CPFTicket;
int cpf_init(CPF *,cmx_kernel *);
int cpf_register(CPF *,unsigned,CPFHandle *);
int cpf_begin(CPFHandle,unsigned mode,cmx_deadline,CPFTicket *);
int cpf_upgrade(CPFHandle,cmx_deadline,CPFTicket *);
int cpf_result(CPFTicket);
int cpf_cancel(CPFTicket);
int cpf_unlock(CPFHandle);
int cpf_close(CPF *);
int cpf_release(CPFHandle *);
int cpf_destroy(CPF *);
#endif
