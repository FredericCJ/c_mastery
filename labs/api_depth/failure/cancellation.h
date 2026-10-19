#ifndef CMASTERY_FAILURE_CANCELLATION_H
#define CMASTERY_FAILURE_CANCELLATION_H
#include "foundation.h"
#include "public_error.h"
typedef void (*AeTerminal)(void *, AeCode, unsigned generation,
                           const unsigned char *, unsigned);
typedef struct {
  FAsync backend;
  AeTerminal terminal;
  void *context;
  unsigned char payload[4];
  unsigned length, timed_out, reported;
} AeOperation;
typedef enum { AE_DEADLINE, AE_CANCEL_ACK, AE_COMPLETE, AE_JOINED } AeEvent;
/* Storage starts zeroed. All core calls serialized. callback may destroy op.
 * ACK/JOINED assert no producer will access retained payload after this event.
 * COMPLETE also relinquishes that access. Generation is bounded to1..3. */
AeCode ae_operation_start(AeOperation *, const unsigned char *, unsigned,
                          AeTerminal, void *, unsigned *generation);
AeCode ae_operation_event(AeOperation *, unsigned generation, AeEvent);
#endif
