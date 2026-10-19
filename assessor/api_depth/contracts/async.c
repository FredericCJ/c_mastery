#include "async.h"
struct AcBox { AcAllocate allocator; size_t length; unsigned char data[8]; };
void ac_operation_init(AcOperation *operation) {
  if (operation) {
    operation->core = (FAsync){F_IDLE,0,0};
    operation->box = 0; operation->terminal = 0; operation->context = 0;
    operation->committed = operation->reporting = 0;
  }
}
const unsigned char *ac_box_data(const AcBox *box, size_t *count) {
  if (!box || !count) return 0;
  *count = box->length; return box->data;
}
void ac_box_drop(AcBox *box) {
  if (box) { AcAllocate allocator = box->allocator;
    allocator.release(allocator.context, box); }
}
static int active(const AcOperation *operation, uint32_t ticket) {
  return operation && !operation->reporting &&
      ticket == operation->core.generation &&
      (operation->core.state == F_INFLIGHT ||
       operation->core.state == F_CANCEL_REQUESTED);
}
AcAsyncStatus ac_submit_v2(AcOperation *operation, const unsigned char *input,
                          size_t count, AcAllocate allocator,
                          AcTerminal terminal, void *context,
                          unsigned inline_finish, uint32_t *ticket) {
  if (!operation || (count && !input) || count > 8 || !terminal || !ticket ||
      !allocator.allocate || !allocator.release || inline_finish > 1)
    return ACX_INVALID;
  if (operation->reporting || operation->core.state == F_INFLIGHT ||
      operation->core.state == F_CANCEL_REQUESTED) return ACX_BUSY;
  if (operation->core.generation == UINT32_MAX ||
      operation->core.releases == UINT32_MAX) return ACX_EXHAUSTED;
  AcBox *box = allocator.allocate(allocator.context,sizeof *box,_Alignof(AcBox));
  if (!box) return ACX_ALLOC;
  box->allocator = allocator; box->length = count;
  for (size_t i=0;i<count;++i) box->data[i]=input[i]^0x20u;
  uint32_t submitted;
  if (f_submit(&operation->core,&submitted)!=F_OK) {
    ac_box_drop(box); return ACX_INVALID;
  }
  operation->box=box; operation->terminal=terminal; operation->context=context;
  operation->committed=0; *ticket=submitted;
  if (inline_finish) return ac_finish_v2(operation,submitted,0);
  return ACX_OK;
}
AcAsyncStatus ac_cancel_v2(AcOperation *operation,uint32_t ticket) {
  if (!active(operation,ticket)) return ACX_INVALID;
  if (operation->core.state==F_INFLIGHT &&
      f_cancel(&operation->core,ticket)!=F_PENDING) return ACX_INVALID;
#ifdef AC_MUTANT_EARLY_CANCEL
  return ac_finish_v2(operation,ticket,1);
#else
  return operation->committed ? ACX_INDETERMINATE : ACX_PENDING;
#endif
}
AcAsyncStatus ac_commit_v2(AcOperation *operation,uint32_t ticket) {
  if (!active(operation,ticket) || operation->committed) return ACX_INVALID;
  operation->committed=1; return ACX_OK;
}
AcAsyncStatus ac_finish_v2(AcOperation *operation,uint32_t ticket,unsigned ack) {
  if (!active(operation,ticket) || ack>1 ||
      (ack && operation->core.state!=F_CANCEL_REQUESTED)) return ACX_INVALID;
  AcAsyncStatus result = ack ? (operation->committed ? ACX_INDETERMINATE :
                                                ACX_CANCELLED) : ACX_OK;
#ifdef AC_MUTANT_ROLLBACK
  if (ack) result=ACX_CANCELLED;
#endif
  if (f_complete(&operation->core,ticket)!=F_OK) return ACX_EXHAUSTED;
  AcBox *box=operation->box; AcTerminal terminal=operation->terminal;
  void *context=operation->context;
  operation->box=0; operation->terminal=0; operation->context=0;
  operation->reporting=1;
  if (ack) { ac_box_drop(box); box=0; }
  terminal(context,result,box); /* Recipient may immediately destroy the box. */
  operation->reporting=0; /* Operation control must live until outer call returns. */
  return ACX_OK;
}
