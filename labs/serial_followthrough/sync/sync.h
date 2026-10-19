#ifndef SERIAL_SYNC_FOLLOWTHROUGH_H
#define SERIAL_SYNC_FOLLOWTHROUGH_H
#include "uart.h"
#include "serial_depth.h"
enum { SU_UNSUPPORTED = 20 };
typedef struct {
  PuDriver byte;
  PuRx receive;
  uint32_t token, deadline, last;
  unsigned configured, cpol, cpha, lbcl, active, count, position, received,
      byte_received, stopping, result;
  const uint8_t *tx;
  PuByte *rx;
} Su;
/* Clock/gate/reset initialization is the existing pu_open operation. */
int su_configure(Su *, PuIo, unsigned cpol, unsigned cpha, unsigned lbcl);
int su_enable(Su *, PuIo, unsigned transmit, unsigned receive);
int su_begin(Su *, PuIo, uint32_t generation, uint32_t token,
             const uint8_t *, unsigned count, PuByte *, unsigned capacity,
             uint32_t now, uint32_t budget);
int su_step(Su *, PuIo, uint32_t token, uint32_t now);
int su_cancel(Su *, PuIo, uint32_t token);
/* Diagnose a synthetic complete trace against an independently declared mode. */
int su_trace(unsigned cpol, unsigned cpha, unsigned lbcl, const SDSync *,
             unsigned *first_bad_edge);
/* F439-only CK pin, PD10/AF7; exclusive port ownership and board ledger required. */
void su_f439_ck(void);
#endif
