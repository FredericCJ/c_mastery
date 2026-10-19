#ifndef PL_COMPARE_H
#define PL_COMPARE_H
#include "service.h"
typedef uint32_t (*PlTick)(void *);
typedef struct {
  volatile uint32_t *status, *data;
  uint32_t txe, tc;
  PlTick tick;
  void *context;
} PlBytePort;
/* Selected preconfigured 8N1 one-byte operation, timeout in progressing ticks.
 * On TIMEOUT, memory is no longer borrowed but the line may still be owned. */
int pl_byte(PlBytePort, uint8_t, uint32_t timeout);
PlBytePort pl_direct_f439(volatile uint32_t *, PlTick, void *);
PlBytePort pl_direct_g474(volatile uint32_t *, PlTick, void *);
PlBytePort pl_generated(volatile uint32_t *, unsigned revision, PlTick, void *);
#endif
