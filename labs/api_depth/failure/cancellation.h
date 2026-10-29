#ifndef CMASTERY_FAILURE_CANCELLATION_H
#define CMASTERY_FAILURE_CANCELLATION_H
#include "foundation.h"
#include "public_error.h"
typedef void (*ae_terminal_fn)(void *, enum ae_code, unsigned generation,
                           const unsigned char *, unsigned);
struct ae_operation {
  struct f_async backend;
  ae_terminal_fn terminal;
  void *context;
  unsigned char payload[4];
  unsigned length, timed_out, reported;
};
enum ae_event { AE_DEADLINE, AE_CANCEL_ACK, AE_COMPLETE, AE_JOINED };
/* Storage starts zeroed. All core calls serialized. callback may destroy op.
 * ACK/JOINED assert no producer will access retained payload after this event.
 * COMPLETE also relinquishes that access. Generation is bounded to1..3. */
enum ae_code ae_operation_start(struct ae_operation *, const unsigned char *, unsigned,
                          ae_terminal_fn, void *, unsigned *generation);
enum ae_code ae_operation_event(struct ae_operation *, unsigned generation, enum ae_event);
#endif
