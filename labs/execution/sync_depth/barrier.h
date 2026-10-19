#ifndef CMASTERY_BARRIER_DEPTH_H
#define CMASTERY_BARRIER_DEPTH_H
#include "cmx.h"
enum { CDB_IDLE, CDB_PENDING, CDB_DONE };
enum { CDB_STALE=CMX_UNIMPLEMENTED+1, CDB_RETIRED };
typedef struct {
  unsigned state, round, task_generation;
  int result;
} CDBSlot;
typedef struct {
  cmx_kernel *kernel;
  cmx_barrier raw;
  CDBSlot slot[CMX_TASKS];
  uint32_t members, registered, next_members, epoch;
  unsigned generation_limit, changing, live;
} CDBarrier;
typedef struct { CDBarrier *owner; unsigned task; uint32_t epoch; } CDBMember;
typedef struct { CDBMember member; unsigned round; } CDBTicket;
int cdb_init(CDBarrier *,cmx_kernel *,uint32_t members,unsigned generation_limit);
int cdb_register(CDBarrier *,unsigned task,CDBMember *);
int cdb_arrive(CDBMember,cmx_deadline,CDBTicket *);
int cdb_result(CDBTicket,unsigned *completed_round);
int cdb_cancel(CDBTicket);
int cdb_stage(CDBarrier *,uint32_t next_members);
int cdb_release(CDBMember *);
int cdb_apply(CDBarrier *);
int cdb_close(CDBarrier *);
int cdb_destroy(CDBarrier *);
#endif
