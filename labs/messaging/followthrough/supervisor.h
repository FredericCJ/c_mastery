#ifndef CMASTERY_MESSAGING_SUPERVISOR_FOLLOW_H
#define CMASTERY_MESSAGING_SUPERVISOR_FOLLOW_H
#include "owned.h"
typedef bool (*MfsOpen)(void *,unsigned,unsigned);
typedef void (*MfsClose)(void *,unsigned,unsigned);
typedef struct {
  MsgSupervisor parent, children[4];
  MfaActor actors[4][4];
  bool resources[4][4], closed;
  MfsOpen open;
  MfsClose close;
  void *resource_ctx;
  size_t child_limit;
  uint64_t window,last;
} MfsTree;
typedef struct {
  int events[64]; size_t count;
  unsigned attempts;
  bool parent_used, rollback, closed;
} MfsReport;
MsgStatus mfs_init(MfsTree *,size_t,size_t,uint64_t,MfsOpen,MfsClose,void *,
                   MfaHandler,void *[4][4],MfaDeadLetter,void *);
/* failure_mask bits0..7 select the actual constructor attempts within this call. */
MsgStatus mfs_fault(MfsTree *,unsigned,unsigned,uint64_t,MsgStrategy,unsigned,MfsReport *);
MsgStatus mfs_destroy(MfsTree *);
#endif
