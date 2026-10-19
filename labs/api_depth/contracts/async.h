#ifndef CMASTERY_CONTRACT_ASYNC_H
#define CMASTERY_CONTRACT_ASYNC_H
#include "evolution_v1.h"
#include "session_v1.h"
#include "foundation.h"
typedef enum { ACX_OK, ACX_INVALID, ACX_BUSY, ACX_ALLOC, ACX_PENDING,
               ACX_CANCELLED, ACX_INDETERMINATE, ACX_EXHAUSTED,
               ACX_UNIMPLEMENTED } AcAsyncStatus;
typedef struct AcBox AcBox;
typedef void (*AcTerminal)(void *, AcAsyncStatus, AcBox *);
typedef struct {
  FAsync core;
  AcBox *box;
  AcTerminal terminal;
  void *context;
  unsigned committed, reporting;
} AcOperation;
void ac_operation_init(AcOperation *);
AcAsyncStatus ac_submit_v2(AcOperation *, const unsigned char *, size_t,
                          AcAllocate, AcTerminal, void *, unsigned inline_finish,
                          uint32_t *ticket);
AcAsyncStatus ac_cancel_v2(AcOperation *, uint32_t);
AcAsyncStatus ac_commit_v2(AcOperation *, uint32_t);
AcAsyncStatus ac_finish_v2(AcOperation *, uint32_t, unsigned cancel_ack);
const unsigned char *ac_box_data(const AcBox *, size_t *);
void ac_box_drop(AcBox *);
#endif
